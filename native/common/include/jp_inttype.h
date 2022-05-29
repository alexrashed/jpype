/*****************************************************************************
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   See NOTICE file for details.
 *****************************************************************************/
#ifndef _JP_INT_TYPE_H_
#define _JP_INT_TYPE_H_

class JPIntType : public JPPrimitiveType
{
public:

	JPIntType();
	~JPIntType() override;

public:
	typedef jint type_t;
	typedef jintArray array_t;

	static inline jint& field(jvalue& v)
	{
		return v.i;
	}

	static inline const jint& field(const jvalue& v)
	{
		return v.i;
	}

	JPClass* getBoxedClass(JPContext *context) const override
	{
		return context->_java_lang_Integer;
	}

	JPMatch::Type findJavaConversion(JPMatch& match) override;
	void getConversionInfo(JPConversionInfo &info) override;
	JPPyObject  convertToPythonObject(JPJavaFrame& frame, jvalue val, bool cast) override;
	JPValue     getValueFromObject(const JPValue& obj) override;

	JPPyObject  invokeStatic(JPJavaFrame& frame, jclass, jmethodID, jvalue*) override;
	JPPyObject  invoke(JPJavaFrame& frame, jobject, jclass, jmethodID, jvalue*) override;

	JPPyObject  getStaticField(JPJavaFrame& frame, jclass c, jfieldID fid) override;
	void        setStaticField(JPJavaFrame& frame, jclass c, jfieldID fid, PyObject* val) override;
	JPPyObject  getField(JPJavaFrame& frame, jobject c, jfieldID fid) override;
	void        setField(JPJavaFrame& frame, jobject c, jfieldID fid, PyObject* val) override;

	jarray      newArrayOf(JPJavaFrame& frame, jsize size) override;
	void        setArrayRange(JPJavaFrame& frame, jarray,
			jsize start, jsize length, jsize step,
			PyObject* sequence) override;
	JPPyObject  getArrayItem(JPJavaFrame& frame, jarray, jsize ndx) override;
	void        setArrayItem(JPJavaFrame& frame, jarray, jsize ndx, PyObject* val) override;

	char getTypeCode() override
	{
		return 'I';
	}

	jlong getAsLong(jvalue v) override  // GCOVR_EXCL_LINE
	{
		return field(v);  // GCOVR_EXCL_LINE
	}

	jdouble getAsDouble(jvalue v) override
	{
		return field(v);
	}

	static jlong assertRange(const jlong& l)
	{
		if (l < -2147483648ll || l > 2147483647ll)
		{
			JP_RAISE(PyExc_OverflowError, "Cannot convert value to Java int");
		}
		return l;
	}

	void getView(JPArrayView& view) override;
	void releaseView(JPArrayView& view) override;
	const char* getBufferFormat() override;
	Py_ssize_t getItemSize() override;
	void copyElements(JPJavaFrame &frame,
			jarray a, jsize start, jsize len,
			void* memory, int offset) override;

	PyObject *newMultiArray(JPJavaFrame &frame,
			JPPyBuffer &buffer, int subs, int base, jobject dims) override;

} ;

#endif // _JP_INT_TYPE_H_
