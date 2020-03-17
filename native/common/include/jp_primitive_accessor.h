#ifndef JP_PRIMITIVE_ACCESSOR_H
#define JP_PRIMITIVE_ACCESSOR_H

#include "jp_exception.h"
#include "jp_javaframe.h"

template <typename array_t, typename ptr_t>
class JPPrimitiveArrayAccessor
{
	typedef void (JPJavaFrame::*releaseFnc)(array_t, ptr_t, jint);
	typedef ptr_t (JPJavaFrame::*accessFnc)(array_t, jboolean*);

	JPJavaFrame& _frame;
	array_t _array;
	ptr_t _elem;
	releaseFnc _release;
	jboolean _iscopy;

public:

	JPPrimitiveArrayAccessor(JPJavaFrame& frame, jarray array, accessFnc access, releaseFnc release)
	: _frame(frame), _array((array_t) array), _release(release)
	{
		_elem = ((&_frame)->*access)(_array, &_iscopy);
	}

	~JPPrimitiveArrayAccessor()
	{
		// This is fallback if commit or abort is not called.
		// It should only occur in cases where a throw has
		// already been issued.
		try
		{
			if (_array)
				((&_frame)->*_release)(_array, _elem, JNI_ABORT);
		}		catch (JPypeException &ex)
		{
			// We can't throw here because it would abort.
			// But this is called on a non-op release, so
			// we will just eat it
		}
	}

	ptr_t get()
	{
		return _elem;
	}

	void commit()
	{
		// Prevent the dtor from calling a second time
		array_t a = _array;
		_array = 0;
		((&_frame)->*_release)(a, _elem, 0);
	}

	void abort()
	{
		// Prevent the dtor from calling a second time
		array_t a = _array;
		_array = 0;
		((&_frame)->*_release)(a, _elem, JNI_ABORT);
	}

};

template <class type_t> PyObject *convertMultiArray(
		JPJavaFrame &frame,
		JPPrimitiveType* cls,
		void (*pack)(type_t*, jvalue),
		const char* code,
		JPPyBuffer &buffer,
		int subs, int base, jobject dims)
{
	JPContext *context = frame.getContext();
	Py_buffer& view = buffer.getView();
	jconverter converter = getConverter(view.format, view.itemsize, code);
	if (converter == NULL)
	{
		PyErr_Format(PyExc_TypeError, "No type converter found");
		return NULL;
	}

	// Reserve space for array.
	jobjectArray contents = (jobjectArray) context->_java_lang_Object->newArrayInstance(frame, subs);
	std::vector<Py_ssize_t> indices(view.ndim);
	int u = view.ndim - 1;
	int k = 0;
	jarray a0 = cls->newArrayInstance(frame, base);
	frame.SetObjectArrayElement(contents, k++, a0);
	jboolean isCopy;
	void *mem = frame.getEnv()->GetPrimitiveArrayCritical(a0, &isCopy);
	type_t *dest = (type_t*) mem;

	int step;
	if (view.strides == NULL)
		step = view.itemsize;
	else
		step = view.strides[u];

	// Align with the first element in the array
	char *src = buffer.getBufferPtr(indices);

	// Traverse the array
	while (true)
	{
		if (indices[u] == view.shape[u])
		{
			int j;
			for (j = 0; j < u; ++j)
			{
				indices[u - j - 1]++;
				if (indices[u - j - 1] < view.shape[u - j - 1])
					break;
				indices[u - j - 1] = 0;
			}
			// Commit the current section
			indices[u] = 0;
			frame.getEnv()->ReleasePrimitiveArrayCritical(a0, mem, JNI_COMMIT);
			frame.DeleteLocalRef(a0);

			// If we hit the shape of the uppermost we are done
			if (j == u)
				break;

			a0 = cls->newArrayInstance(frame, base);
			frame.SetObjectArrayElement(contents, k++, a0);
			mem = frame.getEnv()->GetPrimitiveArrayCritical(a0, &isCopy);
			dest = (type_t*) mem;
			src = buffer.getBufferPtr(indices);
		}
		pack(dest, converter(src));
		src += step;
		dest++;
		indices[u]++;
	}

	// Assemble it into a multidimensional array
	jobject out = frame.assemble(dims, contents);

	// Convert it to Python
	JPClass *type = context->_java_lang_Object;
	if (out != NULL)
		type = frame.findClassForObject(out);
	jvalue v;
	v.l = out;
	return type->convertToPythonObject(frame, v).keep();
}


template <typename base_t>
class JPConversionLong : public JPConversion
{
public:

	virtual jvalue convert(JPJavaFrame *frame, JPClass *cls, PyObject *pyobj) override
	{
		jvalue res;
		jlong val = PyLong_AsLongLong(pyobj);
		if (val == -1)
			JP_PY_CHECK();
		base_t::field(res) = (typename base_t::type_t) base_t::assertRange(val);
		return res;
	}
} ;

template <typename base_t>
class JPConversionLongNumber : public JPConversion
{
public:

	virtual jvalue convert(JPJavaFrame *frame, JPClass *cls, PyObject *pyobj) override
	{
		jvalue res;
		PyObject *obj = PyNumber_Long(pyobj);
		JP_PY_CHECK();
		jlong val = PyLong_AsLongLong(obj);
		Py_DECREF(obj);
		if (val == -1)
			JP_PY_CHECK();
		base_t::field(res) = (typename base_t::type_t) base_t::assertRange(val);
		return res;
	}
};

extern "C" JPValue* PyJPValue_getJavaSlot(PyObject* self);

template <typename base_t>
class JPConversionLongWiden : public JPConversion
{
public:

	virtual jvalue convert(JPJavaFrame *frame, JPClass *cls, PyObject *pyobj) override
	{
		JPValue *value = PyJPValue_getJavaSlot(pyobj);
		jvalue ret;
		base_t::field(ret) = (typename base_t::type_t) ((JPPrimitiveType*) value->getClass())->getAsLong(value->getValue());
		return ret;
	}
};


#endif /* JP_PRIMITIVE_ACCESSOR_H */