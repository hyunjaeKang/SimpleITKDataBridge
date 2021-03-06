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
#include "itkImage.h"
#include "itkVectorImage.h"

namespace sitk = itk::simple;

template<typename TPixel, unsigned int VImageDimension = 2>
const void
sitkSetReferenceCountOfScalarImage(sitk::Image * sitkImage,
                                   bool bIncreaseRefCnt = true)
{
  typedef itk::Image<TPixel, VImageDimension>  ImageType;
  typename ImageType::Pointer itkImage = (ImageType*)(sitkImage->GetITKBase());
  if(bIncreaseRefCnt)
    {
    itkImage->GetPixelContainer()->Register();
    }
  else
    {
    if(itkImage->GetPixelContainer()->GetReferenceCount() > 1)
      {
      itkImage->GetPixelContainer()->UnRegister();
      }
    }
}

template<typename TPixel, unsigned int VImageDimension = 2>
const void*
sitkSetReferenceCountOfVectorImage(sitk::Image * sitkImage,
                                   bool bIncreaseRefCnt = true)
{
  typedef itk::VectorImage<TPixel, VImageDimension>  ImageType;
  typename ImageType::Pointer itkImage = (ImageType*)(sitkImage->GetITKBase());
   if(bIncreaseRefCnt)
    {
    itkImage->GetPixelContainer()->Register();
    }
  else
    {
    if(itkImage->GetPixelContainer()->GetReferenceCount() > 1)
      {
      itkImage->GetPixelContainer()->UnRegister();
      }
    }
}

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
  sitk::Image *               sitkImage;
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
  dimension = sitkImage->GetDimension();
  size      = sitkImage->GetSize();

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
  PyObject *                  pyImageObj    = NULL;
  PyObject *                  obj           = NULL;
  PyObject *                  shapeseq      = NULL;
  PyObject *                  item          = NULL;

  Py_ssize_t                  buffer_len;
  Py_buffer                   pyBuffer;
  memset(&pyBuffer, 0, sizeof(Py_buffer));

  const void *                buffer;
  void *                      sitkBufferPtr = NULL;
  sitk::Image *               sitkImage     = NULL;
  sitk::ImportImageFilter     importer;

  int                         arrayViewFlag = 0;
  int                         PixelIDValue  = 0;
  int                         NumOfComponent= 1;
  unsigned int                dimension     = 0;


  size_t                      pixelSize     = 1;
  size_t                      len           = 1;
  std::vector< unsigned int > size;
  std::vector< double>        spacing;
  std::vector< double>        origin;
  std::vector< double>        direction;

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
    }

  shapeseq   = PySequence_Fast(obj, "expected sequence");
  dimension  = PySequence_Size(obj);

  for(unsigned int i=0 ; i< dimension; ++i)
    {
    item = PySequence_Fast_GET_ITEM(shapeseq,i);
    size.push_back((unsigned int)PyInt_AsLong(item));
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

  if(arrayViewFlag == 0)
    {
    sitkImage  = new itk::simple::Image(size, (itk::simple::PixelIDValueEnum)PixelIDValue, NumOfComponent);
    }
  else
    {
    importer.SetSize( size );
    importer.SetSpacing( spacing );
    importer.SetOrigin( origin );
    importer.SetDirection( direction );
    }

  try
    {
    switch( PixelIDValue )
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
    }
  else
    {
    sitkImage  = new itk::simple::Image(static_cast< const itk::simple::Image& >(importer.Execute()));
    }

  PyBuffer_Release( &pyBuffer );
  pyImageObj = SWIG_NewPointerObj(sitkImage, SWIGTYPE_p_itk__simple__Image, SWIG_POINTER_OWN |  0 );
  return pyImageObj;

fail:
  PyBuffer_Release( &pyBuffer );
  return NULL;
}

/** An internal function that performs a deep copy of the image buffer
 * into a python byte array. The byte array can later be converted
 * into a numpy array with the from buffer method.
 */
static PyObject *
sitk_SetRefenceCountImage( PyObject *SWIGUNUSEDPARM(self), PyObject *args )
{
  const void *                sitkBufferPtr;

  size_t                      pixelSize     = 1;

  unsigned int                dimension;

  /* Cast over to a sitk Image. */
  PyObject *                  pyImage;
  void *                      voidImage;
  sitk::Image *               sitkImage;
  int                         res           = 0;
  int                         arrayViewFlag = 0;


  bool                        bIncreaseRefCntOfitkImage = true;

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

  dimension = sitkImage->GetDimension();

  if(arrayViewFlag == 0)
    {
    bIncreaseRefCntOfitkImage = false;
    }
  else if (arrayViewFlag == 1)
    {
    bIncreaseRefCntOfitkImage = true;
    }
  else
    {
    PyErr_SetString( PyExc_RuntimeError, "Wrong conversion operation." );
    SWIG_fail;
    }

  switch( sitkImage->GetPixelIDValue() )
    {
  case sitk::sitkUnknown:
    PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
    SWIG_fail;
    break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt8 != sitk::sitkUnknown, sitk::sitkVectorUInt8, -14 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<uint8_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<uint8_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( uint8_t );
    break;
  case sitk::ConditionalValue< sitk::sitkUInt8 != sitk::sitkUnknown, sitk::sitkUInt8, -2 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<uint8_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<uint8_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( uint8_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorInt8 != sitk::sitkUnknown, sitk::sitkVectorInt8, -15 >::Value:
      if(dimension == 2)
      {
       sitkSetReferenceCountOfVectorImage<int8_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<int8_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( int8_t );
    break;
  case sitk::ConditionalValue< sitk::sitkInt8 != sitk::sitkUnknown, sitk::sitkInt8, -3 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<int8_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<int8_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( int8_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt16 != sitk::sitkUnknown, sitk::sitkVectorUInt16, -16 >::Value:
      if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<uint16_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<uint16_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( uint16_t );
    break;
  case sitk::ConditionalValue< sitk::sitkUInt16 != sitk::sitkUnknown, sitk::sitkUInt16, -4 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<uint16_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<uint16_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( uint16_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorInt16 != sitk::sitkUnknown, sitk::sitkVectorInt16, -17 >::Value:
      if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<int16_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<int16_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( int16_t );
    break;
  case sitk::ConditionalValue< sitk::sitkInt16 != sitk::sitkUnknown, sitk::sitkInt16, -5 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<int16_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<int16_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( int16_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt32 != sitk::sitkUnknown, sitk::sitkVectorUInt32, -18 >::Value:
      if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<uint32_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<uint32_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( uint32_t );
    break;
  case sitk::ConditionalValue< sitk::sitkUInt32 != sitk::sitkUnknown, sitk::sitkUInt32, -6 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<uint32_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<uint32_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( uint32_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorInt32 != sitk::sitkUnknown, sitk::sitkVectorInt32, -19 >::Value:
      if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<int32_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<int32_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( int32_t );
    break;
  case sitk::ConditionalValue< sitk::sitkInt32 != sitk::sitkUnknown, sitk::sitkInt32, -7 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<int32_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<int32_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( int32_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorUInt64 != sitk::sitkUnknown, sitk::sitkVectorUInt64, -20 >::Value:
      if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<uint64_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<uint64_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( uint64_t );
    break;
  case sitk::ConditionalValue< sitk::sitkUInt64 != sitk::sitkUnknown, sitk::sitkUInt64, -8 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<uint64_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<uint64_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( uint64_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorInt64 != sitk::sitkUnknown, sitk::sitkVectorInt64, -21 >::Value:
      if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<int64_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<int64_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( int64_t );
    break;
  case sitk::ConditionalValue< sitk::sitkInt64 != sitk::sitkUnknown, sitk::sitkInt64, -9 >::Value:
     if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<int64_t, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<int64_t, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( int64_t );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorFloat32 != sitk::sitkUnknown, sitk::sitkVectorFloat32, -22 >::Value:
      if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<float, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<float, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( float );
    break;
  case sitk::ConditionalValue< sitk::sitkFloat32 != sitk::sitkUnknown, sitk::sitkFloat32, -10 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<float, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<float, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( float );
    break;
  case sitk::ConditionalValue< sitk::sitkVectorFloat64 != sitk::sitkUnknown, sitk::sitkVectorFloat64, -23 >::Value:
      if(dimension == 2)
      {
      sitkSetReferenceCountOfVectorImage<double, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfVectorImage<double, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
    pixelSize  = sizeof( double );
    break;
  case sitk::ConditionalValue< sitk::sitkFloat64 != sitk::sitkUnknown, sitk::sitkFloat64, -11 >::Value:
    if(dimension == 2)
      {
      sitkSetReferenceCountOfScalarImage<double, 2>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else if (dimension == 3)
      {
      sitkSetReferenceCountOfScalarImage<double, 3>(sitkImage, bIncreaseRefCntOfitkImage);
      }
    else
      {
      PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
      goto fail;
      }
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

  Py_RETURN_NONE;

fail:
  return NULL;

}

#ifdef __cplusplus
} // end extern "C"
#endif
