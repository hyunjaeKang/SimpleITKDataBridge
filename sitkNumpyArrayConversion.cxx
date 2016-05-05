/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#include <string.h>

#include <numeric>
#include <functional>

#include "sitkImage.h"
#include "sitkConditional.h"
#include "sitkExceptionObject.h"

namespace sitk = itk::simple;

// Python is written in C
#ifdef __cplusplus
extern "C"
{
#endif

/** An internal function that performs a deep copy of the image buffer
 * into a python byte array. The byte array can later be converted
 * into a numpy array with the from buffer method.
 */
static PyObject *
sitk_GetByteArrayFromImage( PyObject *SWIGUNUSEDPARM(self), PyObject *args )
{
  // Holds the bulk data
  PyObject *                  byteArray     = NULL;

  const void *                sitkBufferPtr;
  Py_ssize_t                  len;
  std::vector< unsigned int > size;
  size_t                      pixelSize     = 1;

  unsigned int                dimension;

  /* Cast over to a sitk Image. */
  PyObject *                  pyImage;
  void *                      voidImage;
  const sitk::Image *         sitkImage;
  int                         res           = 0;
  int                         arrayViewFlag = 0;

  PyObject *                  memoryView    = NULL;
  Py_buffer                   pyBuffer;
  memset(&pyBuffer, 0, sizeof(Py_buffer));


  if( !PyArg_ParseTuple( args, "Oi", &pyImage, &arrayViewFlag ) )
    {
    SWIG_fail; // SWIG_fail is a macro that says goto: fail (return NULL)
    }
  res = SWIG_ConvertPtr( pyImage, &voidImage, SWIGTYPE_p_itk__simple__Image, 0 );
  if( !SWIG_IsOK( res ) )
    {
    SWIG_exception_fail(SWIG_ArgError(res), "in method 'GetByteArrayFromImage', argument needs to be of type 'sitk::Image *'");
    }
  sitkImage = reinterpret_cast< sitk::Image * >( voidImage );

  switch( sitkImage->GetPixelIDValue() )
    {
  case sitk::sitkUnknown:
    PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
    SWIG_fail;
    break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt8 != sitk::sitkUnknown, sitk::sitkVectorUInt8, -14 >::Value:
  case sitk::ConditionalValue< sitk::sitkUInt8 != sitk::sitkUnknown, sitk::sitkUInt8, -2 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsUInt8();
    pixelSize  = sizeof( uint8_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorInt8 != sitk::sitkUnknown, sitk::sitkVectorInt8, -15 >::Value:
  case sitk::ConditionalValue< sitk::sitkInt8 != sitk::sitkUnknown, sitk::sitkInt8, -3 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsInt8();
    pixelSize  = sizeof( int8_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt16 != sitk::sitkUnknown, sitk::sitkVectorUInt16, -16 >::Value:
  case sitk::ConditionalValue< sitk::sitkUInt16 != sitk::sitkUnknown, sitk::sitkUInt16, -4 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsUInt16();
    pixelSize  = sizeof( uint16_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorInt16 != sitk::sitkUnknown, sitk::sitkVectorInt16, -17 >::Value:
  case sitk::ConditionalValue< sitk::sitkInt16 != sitk::sitkUnknown, sitk::sitkInt16, -5 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsInt16();
    pixelSize  = sizeof( int16_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt32 != sitk::sitkUnknown, sitk::sitkVectorUInt32, -18 >::Value:
  case sitk::ConditionalValue< sitk::sitkUInt32 != sitk::sitkUnknown, sitk::sitkUInt32, -6 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsUInt32();
    pixelSize  = sizeof( uint32_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorInt32 != sitk::sitkUnknown, sitk::sitkVectorInt32, -19 >::Value:
  case sitk::ConditionalValue< sitk::sitkInt32 != sitk::sitkUnknown, sitk::sitkInt32, -7 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsInt32();
    pixelSize  = sizeof( int32_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt64 != sitk::sitkUnknown, sitk::sitkVectorUInt64, -20 >::Value:
  case sitk::ConditionalValue< sitk::sitkUInt64 != sitk::sitkUnknown, sitk::sitkUInt64, -8 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsUInt64();
    pixelSize  = sizeof( uint64_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorInt64 != sitk::sitkUnknown, sitk::sitkVectorInt64, -21 >::Value:
  case sitk::ConditionalValue< sitk::sitkInt64 != sitk::sitkUnknown, sitk::sitkInt64, -9 >::Value:
     sitkBufferPtr = (const void *)sitkImage->GetBufferAsInt64();
     pixelSize  = sizeof( int64_t );
     break;
  case sitk::ConditionalValue< sitk::sitkVectorFloat32 != sitk::sitkUnknown, sitk::sitkVectorFloat32, -22 >::Value:
  case sitk::ConditionalValue< sitk::sitkFloat32 != sitk::sitkUnknown, sitk::sitkFloat32, -10 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsFloat();
    pixelSize  = sizeof( float );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorFloat64 != sitk::sitkUnknown, sitk::sitkVectorFloat64, -23 >::Value:
  case sitk::ConditionalValue< sitk::sitkFloat64 != sitk::sitkUnknown, sitk::sitkFloat64, -11 >::Value:
    sitkBufferPtr = (const void *)sitkImage->GetBufferAsDouble(); // \todo rename to Float64 for consistency
    pixelSize  = sizeof( double );
    break;
  case sitk::ConditionalValue< sitk::sitkComplexFloat32 != sitk::sitkUnknown, sitk::sitkComplexFloat32, -12 >::Value:
  case sitk::ConditionalValue< sitk::sitkComplexFloat64 != sitk::sitkUnknown, sitk::sitkComplexFloat64, -13 >::Value:
    PyErr_SetString( PyExc_RuntimeError, "Images of Complex Pixel types currently are not supported." );
    SWIG_fail;
    break;
  default:
    PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
    SWIG_fail;
    }

  dimension = sitkImage->GetDimension();
  size = sitkImage->GetSize();

  // if the image is a vector just treat is as another dimension
  if ( sitkImage->GetNumberOfComponentsPerPixel() > 1 )
    {
    size.push_back( sitkImage->GetNumberOfComponentsPerPixel() );
    }

  len = std::accumulate( size.begin(), size.end(), size_t(1), std::multiplies<size_t>() );
  len *= pixelSize;

  if(arrayViewFlag == 0)
    {
    // When the string is null, the bytearray is uninitialized but allocated
    byteArray = PyByteArray_FromStringAndSize( NULL, len );
    if( !byteArray )
      {
      PyErr_SetString( PyExc_RuntimeError, "Error initializing bytearray." );
      SWIG_fail;
      }

    char *arrayView;
    if( (arrayView = PyByteArray_AsString( byteArray ) ) == NULL  )
      {
      SWIG_fail;
      }
    memcpy( arrayView, sitkBufferPtr, len );

    return byteArray;
    }
  else if (arrayViewFlag == 1)
    {
    res = PyBuffer_FillInfo(&pyBuffer, NULL, (void*)sitkBufferPtr, len, 0, PyBUF_CONTIG);
    memoryView = PyMemoryView_FromBuffer(&pyBuffer);

    PyBuffer_Release(&pyBuffer);
    return memoryView;
    }
  else
    {
    PyErr_SetString( PyExc_RuntimeError, "Wrong conversion operation." );
    SWIG_fail;
    }

fail:
  Py_XDECREF( byteArray );
  return NULL;
}

/** An internal function that performs a deep copy of the image buffer
 * into a python byte array. The byte array can later be converted
 * into a numpy array with the frombuffer method.
 */
static PyObject*
sitk_SetImageFromArray( PyObject *SWIGUNUSEDPARM(self), PyObject *args )
{
  PyObject *                  pyImage       = NULL;
  PyObject *                  resultobj     = NULL;

  Py_ssize_t                  buffer_len;
  Py_buffer                   pyBuffer;
  memset(&pyBuffer, 0, sizeof(Py_buffer));

  const void *                buffer;
  void *                      sitkBufferPtr = NULL;
  sitk::Image *         sitkImage     = NULL;
  sitk::ImportImageFilter     importer;

  int                         arrayViewFlag = 0;
  int                         PixelIDValue  = 0;
  int                         NumOfComponent= 1;
  unsigned int                numberOfArgs  = PyTuple_Size(args);

  size_t                      pixelSize     = 1;
  unsigned int                dimension     = 0;
  size_t                      len           = 1;
  std::vector< unsigned int > size;
  std::vector< double>        spacing;
  std::vector< double>        origin;
  std::vector< double>        direction;

  if(numberOfArgs < 4)  // Get SimpleITK image buffer with copy operation
    {
    // We wish to support both the new PEP3118 buffer interface and the
    // older. So we first try to parse the arguments with the new buffer
    // protocol, then the old.
    if (!PyArg_ParseTuple( args, "s*iO", &pyBuffer, &arrayViewFlag, &pyImage ) )
      {
      PyErr_Clear();

#ifdef PY_SSIZE_T_CLEAN
      typedef Py_ssize_t bufSizeType;
#else
      typedef int bufSizeType;
#endif

      bufSizeType _len;
      // This function takes 2 arguments from python, the first is an
      // python object which support the old "ReadBuffer" interface
      if( !PyArg_ParseTuple( args, "s#iO", &buffer, &_len, &arrayViewFlag, &pyImage ) )
        {
        return NULL;
        }
      buffer_len = _len;
      }
    else
      {
      if ( PyBuffer_IsContiguous( &pyBuffer, 'C' ) != 1 )
        {
        PyBuffer_Release( &pyBuffer );
        PyErr_SetString( PyExc_TypeError, "A C Contiguous buffer object is required." );
        return NULL;
        }
      buffer_len = pyBuffer.len;
      buffer = pyBuffer.buf;
      }

    /* Cast over to a sitk Image. */
      {
      void * voidImage;
      int res = 0;
      res = SWIG_ConvertPtr( pyImage, &voidImage, SWIGTYPE_p_itk__simple__Image, 0 );
      if( !SWIG_IsOK( res ) )
        {
        SWIG_exception_fail(SWIG_ArgError(res), "in method 'SetImageFromArray', argument needs to be of type 'sitk::Image *'");
        }
      sitkImage = reinterpret_cast< sitk::Image * >( voidImage );
      }

      dimension = sitkImage->GetDimension();
      size = sitkImage->GetSize();
      NumOfComponent = sitkImage->GetNumberOfComponentsPerPixel();
    }
  else
    {

    PyObject * obj      = NULL;
    PyObject * shapeseq = NULL;
    PyObject * item     = NULL;

    if (!PyArg_ParseTuple( args, "s*iOi|i", &pyBuffer, &arrayViewFlag, &obj, &PixelIDValue, &NumOfComponent ))
      {
      PyErr_Clear();

#ifdef PY_SSIZE_T_CLEAN
      typedef Py_ssize_t bufSizeType;
#else
      typedef int bufSizeType;
#endif

      bufSizeType _len;
      if( !PyArg_ParseTuple( args, "s#iOi|i", &buffer, &_len, &arrayViewFlag, &obj, &PixelIDValue, &NumOfComponent ) )
        {
        return NULL;
        }
      buffer_len = _len;
      }
    else
      {
      buffer_len = pyBuffer.len;
      buffer     = pyBuffer.buf;
      shapeseq   = PySequence_Fast(obj, "expected sequence");
      dimension  = PySequence_Size(obj);

      for(unsigned int i=0 ; i< dimension; ++i)
        {
        item = PySequence_Fast_GET_ITEM(shapeseq,i);
        size.push_back((unsigned int)PyInt_AsLong(item));
        }
      }

      if (dimension == 2 )
        {
        spacing      = std::vector<double>( 2, 1.0 );
        origin       = std::vector<double>( 2, 0.0 );
        direction    = std::vector<double>( 4, 0.0 );
        direction[0] = direction[3] = 1.0;
        }
      else if (dimension == 3)
        {
        spacing      = std::vector<double>( 3, 1.0 );
        origin       = std::vector<double>( 3, 0.0 );
        direction    = std::vector<double>( 9, 0.0 );
        direction[0] = direction[4] = direction[8] = 1.0;
        }

      importer.SetSize( size );
      importer.SetSpacing( spacing );
      importer.SetOrigin( origin );
      importer.SetDirection( direction );
    }

  try
    {
    int tempPixelIDValue;
    if (arrayViewFlag == 0)
      {
      tempPixelIDValue = sitkImage->GetPixelIDValue();
      }
    else
      {
      tempPixelIDValue = PixelIDValue;
      }

    switch( tempPixelIDValue )
      {
      case sitk::sitkUnknown:
        PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
        goto fail;
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt8 != sitk::sitkUnknown, sitk::sitkVectorUInt8, -14 >::Value:
      case sitk::ConditionalValue< sitk::sitkUInt8 != sitk::sitkUnknown, sitk::sitkUInt8, -2 >::Value:
        pixelSize         = sizeof( uint8_t );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsUInt8();
          }
        else
          {
          importer.SetBufferAsUInt8((uint8_t*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt8 != sitk::sitkUnknown, sitk::sitkVectorInt8, -15 >::Value:
      case sitk::ConditionalValue< sitk::sitkInt8 != sitk::sitkUnknown, sitk::sitkInt8, -3 >::Value:
        pixelSize         = sizeof( int8_t );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsInt8();
          }
        else
          {
          importer.SetBufferAsInt8((int8_t*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt16 != sitk::sitkUnknown, sitk::sitkVectorUInt16, -16 >::Value:
      case sitk::ConditionalValue< sitk::sitkUInt16 != sitk::sitkUnknown, sitk::sitkUInt16, -4 >::Value:
        pixelSize         = sizeof( uint16_t );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsUInt16();
          }
        else
          {
          importer.SetBufferAsUInt16((uint16_t*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt16 != sitk::sitkUnknown, sitk::sitkVectorInt16, -17 >::Value:
      case sitk::ConditionalValue< sitk::sitkInt16 != sitk::sitkUnknown, sitk::sitkInt16, -5 >::Value:
        pixelSize         = sizeof( int16_t );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsInt16();
          }
        else
          {
          importer.SetBufferAsInt16((int16_t*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt32 != sitk::sitkUnknown, sitk::sitkVectorUInt32, -18 >::Value:
      case sitk::ConditionalValue< sitk::sitkUInt32 != sitk::sitkUnknown, sitk::sitkUInt32, -6 >::Value:
        pixelSize         = sizeof( uint32_t );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsUInt32();
          }
        else
          {
          importer.SetBufferAsUInt32((uint32_t*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt32 != sitk::sitkUnknown, sitk::sitkVectorInt32, -19 >::Value:
      case sitk::ConditionalValue< sitk::sitkInt32 != sitk::sitkUnknown, sitk::sitkInt32, -7 >::Value:
        pixelSize         = sizeof( int32_t );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsInt32();
          }
        else
          {
          importer.SetBufferAsInt32((int32_t*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt64 != sitk::sitkUnknown, sitk::sitkVectorUInt64, -20 >::Value:
      case sitk::ConditionalValue< sitk::sitkUInt64 != sitk::sitkUnknown, sitk::sitkUInt64, -8 >::Value:
        pixelSize         = sizeof( uint64_t );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsUInt64();
          }
        else
          {
          importer.SetBufferAsUInt64((uint64_t*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt64 != sitk::sitkUnknown, sitk::sitkVectorInt64, -21 >::Value:
      case sitk::ConditionalValue< sitk::sitkInt64 != sitk::sitkUnknown, sitk::sitkInt64, -9 >::Value:
        pixelSize         = sizeof( int64_t );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsInt64();
          }
        else
          {
          importer.SetBufferAsInt64((int64_t*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorFloat32 != sitk::sitkUnknown, sitk::sitkVectorFloat32, -22 >::Value:
      case sitk::ConditionalValue< sitk::sitkFloat32 != sitk::sitkUnknown, sitk::sitkFloat32, -10 >::Value:
        pixelSize         = sizeof( float );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsFloat();
          }
        else
          {
          importer.SetBufferAsFloat((float*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorFloat64 != sitk::sitkUnknown, sitk::sitkVectorFloat64, -23 >::Value:
      case sitk::ConditionalValue< sitk::sitkFloat64 != sitk::sitkUnknown, sitk::sitkFloat64, -11 >::Value:
        pixelSize         = sizeof( double );
        if( arrayViewFlag == 0)
          {
          sitkBufferPtr = (void *)sitkImage->GetBufferAsDouble(); // \todo rename to Float64 for consistency
          }
        else
          {
          importer.SetBufferAsDouble((double*)buffer, NumOfComponent);
          }
        break;
      case sitk::ConditionalValue< sitk::sitkComplexFloat32 != sitk::sitkUnknown, sitk::sitkComplexFloat32, -12 >::Value:
      case sitk::ConditionalValue< sitk::sitkComplexFloat64 != sitk::sitkUnknown, sitk::sitkComplexFloat64, -13 >::Value:
        PyErr_SetString( PyExc_RuntimeError, "Images of Complex Pixel types currently are not supported." );
        goto fail;
        break;
      default:
        PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
        goto fail;
      }
    }
  catch( const std::exception &e )
    {
    std::string msg = "Exception thrown in SimpleITK new Image: ";
    msg += e.what();
    PyErr_SetString( PyExc_RuntimeError, msg.c_str() );
    goto fail;
    }

  // if the image is a vector just treat is as another dimension
  if ( NumOfComponent > 1 )
    {
    size.push_back( NumOfComponent );
    }
  len = std::accumulate( size.begin(), size.end(), size_t(1), std::multiplies<size_t>() );
  len *= pixelSize;

  if ( buffer_len != len )
    {
    PyErr_SetString( PyExc_RuntimeError, "Size mismatch of image and Buffer." );
    goto fail;
    }

  if(arrayViewFlag == 0)
    {
    memcpy( (void *)sitkBufferPtr, buffer, len );
    PyBuffer_Release( &pyBuffer );
    Py_RETURN_NONE;
    }
  else
    {
    PyBuffer_Release( &pyBuffer );
    resultobj = SWIG_NewPointerObj((new itk::simple::Image(static_cast< const itk::simple::Image& >(importer.Execute()))), SWIGTYPE_p_itk__simple__Image, SWIG_POINTER_OWN |  0 );
    return resultobj;
    }

fail:
  PyBuffer_Release( &pyBuffer );
  return NULL;
}

#ifdef __cplusplus
} // end extern "C"
#endif
