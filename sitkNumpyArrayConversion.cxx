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

#include "SimpleITK.h"
#include "sitkImage.h"
#include "sitkConditional.h"
#include "sitkExceptionObject.h"

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "itkImportImageFilter.h"
#include "itkDefaultConvertPixelTraits.h"
#include "itkVectorImage.h"

namespace sitk = itk::simple;


template<typename TImage>
int sitkImportImageBuffer(sitk::Image * sitkImage, const void* buffer, size_t numberOfPixels)
{

  typedef TImage                                                                    ImageType;
  typedef typename ImageType::PixelType                                             PixelType;
  typedef typename ImageType::Pointer                                               ImagePointer;

  typedef typename itk::DefaultConvertPixelTraits<PixelType>::ComponentType         ComponentType;
  typedef itk::ImportImageFilter <ComponentType, ImageType::ImageDimension>         ImportImageFilterType;
  typedef typename itk::Image< ComponentType, ImageType::ImageDimension >::Pointer  OutputImagePointer;

  typename ImageType::SizeType            size;
  typename ImageType::IndexType           start;
  typename ImageType::RegionType          region;
  typename ImageType::PointType           origin;
  typename ImageType::SpacingType         spacing;
  typename ImportImageFilterType::Pointer importer = ImportImageFilterType::New();

  start.Fill( 0 );
  size = itk::simple::sitkSTLVectorToITK< typename ImageType::SizeType>( sitkImage->GetSize() );

  region.SetIndex( start );
  region.SetSize( size );
  origin.Fill( 0.0 );
  spacing.Fill( 1.0 );

  importer->SetRegion( region );
  importer->SetOrigin( origin );
  importer->SetSpacing( spacing );

  importer->SetImportPointer( (ComponentType *)buffer,
                              numberOfPixels,
                              true );

  importer->Update();

  OutputImagePointer output = importer->GetOutput();
  output->DisconnectPipeline();

  *sitkImage = sitk::Image(output);

  return 1;
}

template<typename TPixel, unsigned int VImageDimension = 2>
int sitkImportScalarImageBuffer(sitk::Image * sitkImage, const void* buffer, size_t buffer_len)
{
  size_t len                                = 1;
  size_t numberOfPixels                     = 1;
  size_t pixelSize                          = sizeof(TPixel);
  std::vector< unsigned int > sitkImageSize = sitkImage->GetSize();

  // if the image is a vector just treat is as another dimension
  if ( sitkImage->GetNumberOfComponentsPerPixel() > 1 )
    {
    sitkImageSize.push_back( sitkImage->GetNumberOfComponentsPerPixel() );
    }

  numberOfPixels = std::accumulate( sitkImageSize.begin(), sitkImageSize.end(), size_t(1), std::multiplies<size_t>() );
  len = numberOfPixels*pixelSize;

  if ( buffer_len != len )
    {
    PyErr_SetString( PyExc_RuntimeError, "Size mismatch of image and Buffer." );
    return -1;
    }

  typedef itk::Image<TPixel, VImageDimension>                     ITKImageType;

  return sitkImportImageBuffer<ITKImageType>(sitkImage, buffer,numberOfPixels);
}

template<typename TPixel, unsigned int VImageDimension = 2>
int sitkImportVectorImageBuffer(sitk::Image * sitkImage, const void* buffer, size_t buffer_len)
{
  size_t len                                = 1;
  size_t numberOfPixels                     = 1;
  size_t pixelSize                          = sizeof(TPixel);
  std::vector< unsigned int > sitkImageSize = sitkImage->GetSize();

  // if the image is a vector just treat is as another dimension
  if ( sitkImage->GetNumberOfComponentsPerPixel() > 1 )
    {
    sitkImageSize.push_back( sitkImage->GetNumberOfComponentsPerPixel() );
    }

  numberOfPixels = std::accumulate( sitkImageSize.begin(), sitkImageSize.end(), size_t(1), std::multiplies<size_t>() );
  len = numberOfPixels*pixelSize;

  if ( buffer_len != len )
    {
    PyErr_SetString( PyExc_RuntimeError, "Size mismatch of image and Buffer." );
    return -1;
    }

  typedef itk::VectorImage<TPixel, VImageDimension>          ITKVectorImageType;

  return sitkImportImageBuffer<ITKVectorImageType>(sitkImage, buffer,numberOfPixels);
}

//#include "sitkNumpyArrayConversion.h"

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
  PyObject * byteArray = NULL;

  const void * sitkBufferPtr;
  Py_ssize_t len;
  std::vector< unsigned int > size;
  size_t pixelSize = 1;

  unsigned int dimension;

  /* Cast over to a sitk Image. */
  PyObject * pyImage;
  void * voidImage;
  const sitk::Image * sitkImage;
  int res = 0;
  if( !PyArg_ParseTuple( args, "O", &pyImage ) )
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

fail:
  Py_XDECREF( byteArray );
  return NULL;
}

/** An internal function that expose the image buffer
 * into a python byte array. The byte array can later be converted
 * into a numpy array with the from buffer method.
 */
static PyObject *
sitk_GetByteArrayViewFromImage( PyObject *SWIGUNUSEDPARM(self), PyObject *args)
{
  // Holds the bulk data
  PyObject * byteArray = NULL;
  Py_buffer pybuffer;

  const void * sitkBufferPtr;
  Py_ssize_t len;
  std::vector< unsigned int > size;
  size_t pixelSize = 1;

  unsigned int dimension;

  int typenum = 0;

  /* Cast over to a sitk Image. */
  PyObject * pyImage;

  void * voidImage;
  const sitk::Image * sitkImage;
  int res = 0;


  if( !PyArg_ParseTuple( args, "O", &pyImage) )
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

  res = PyBuffer_FillInfo(&pybuffer, NULL, (void*)sitkBufferPtr, len, 0, PyBUF_CONTIG);
  byteArray = PyMemoryView_FromBuffer(&pybuffer);

  PyBuffer_Release(&pybuffer);
  return byteArray;

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
  PyObject * pyImage = NULL;

  const void *buffer;
  Py_ssize_t buffer_len;
  Py_buffer  pyBuffer;
  memset(&pyBuffer, 0, sizeof(Py_buffer));

  const sitk::Image * sitkImage = NULL;
  void * sitkBufferPtr = NULL;
  size_t pixelSize = 1;

  unsigned int dimension = 0;
  std::vector< unsigned int > size;
  size_t len = 1;

  // We wish to support both the new PEP3118 buffer interface and the
  // older. So we first try to parse the arguments with the new buffer
  // protocol, then the old.
  if (!PyArg_ParseTuple( args, "s*O", &pyBuffer, &pyImage ) )
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
    if( !PyArg_ParseTuple( args, "s#O", &buffer, &_len, &pyImage ) )
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

  try
    {
    switch( sitkImage->GetPixelIDValue() )
      {
      case sitk::sitkUnknown:
        PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
        goto fail;
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt8 != sitk::sitkUnknown, sitk::sitkVectorUInt8, -14 >::Value:
      case sitk::ConditionalValue< sitk::sitkUInt8 != sitk::sitkUnknown, sitk::sitkUInt8, -2 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsUInt8();
        pixelSize  = sizeof( uint8_t );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt8 != sitk::sitkUnknown, sitk::sitkVectorInt8, -15 >::Value:
      case sitk::ConditionalValue< sitk::sitkInt8 != sitk::sitkUnknown, sitk::sitkInt8, -3 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsInt8();
        pixelSize  = sizeof( int8_t );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt16 != sitk::sitkUnknown, sitk::sitkVectorUInt16, -16 >::Value:
      case sitk::ConditionalValue< sitk::sitkUInt16 != sitk::sitkUnknown, sitk::sitkUInt16, -4 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsUInt16();
        pixelSize  = sizeof( uint16_t );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt16 != sitk::sitkUnknown, sitk::sitkVectorInt16, -17 >::Value:
      case sitk::ConditionalValue< sitk::sitkInt16 != sitk::sitkUnknown, sitk::sitkInt16, -5 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsInt16();
        pixelSize  = sizeof( int16_t );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt32 != sitk::sitkUnknown, sitk::sitkVectorUInt32, -18 >::Value:
      case sitk::ConditionalValue< sitk::sitkUInt32 != sitk::sitkUnknown, sitk::sitkUInt32, -6 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsUInt32();
        pixelSize  = sizeof( uint32_t );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt32 != sitk::sitkUnknown, sitk::sitkVectorInt32, -19 >::Value:
      case sitk::ConditionalValue< sitk::sitkInt32 != sitk::sitkUnknown, sitk::sitkInt32, -7 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsInt32();
        pixelSize  = sizeof( int32_t );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt64 != sitk::sitkUnknown, sitk::sitkVectorUInt64, -20 >::Value:
      case sitk::ConditionalValue< sitk::sitkUInt64 != sitk::sitkUnknown, sitk::sitkUInt64, -8 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsUInt64();
        pixelSize  = sizeof( uint64_t );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt64 != sitk::sitkUnknown, sitk::sitkVectorInt64, -21 >::Value:
      case sitk::ConditionalValue< sitk::sitkInt64 != sitk::sitkUnknown, sitk::sitkInt64, -9 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsInt64();
        pixelSize  = sizeof( int64_t );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorFloat32 != sitk::sitkUnknown, sitk::sitkVectorFloat32, -22 >::Value:
      case sitk::ConditionalValue< sitk::sitkFloat32 != sitk::sitkUnknown, sitk::sitkFloat32, -10 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsFloat();
        pixelSize  = sizeof( float );
        break;
      case sitk::ConditionalValue< sitk::sitkVectorFloat64 != sitk::sitkUnknown, sitk::sitkVectorFloat64, -23 >::Value:
      case sitk::ConditionalValue< sitk::sitkFloat64 != sitk::sitkUnknown, sitk::sitkFloat64, -11 >::Value:
        sitkBufferPtr = (void *)sitkImage->GetBufferAsDouble(); // \todo rename to Float64 for consistency
        pixelSize  = sizeof( double );
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


  dimension = sitkImage->GetDimension();
  size = sitkImage->GetSize();

  // if the image is a vector just treat is as another dimension
  if ( sitkImage->GetNumberOfComponentsPerPixel() > 1 )
    {
    size.push_back( sitkImage->GetNumberOfComponentsPerPixel() );
    }

  len = std::accumulate( size.begin(), size.end(), size_t(1), std::multiplies<size_t>() );
  len *= pixelSize;

  if ( buffer_len != len )
    {
    PyErr_SetString( PyExc_RuntimeError, "Size mismatch of image and Buffer." );
    goto fail;
    }

  memcpy( (void *)sitkBufferPtr, buffer, len );


  PyBuffer_Release( &pyBuffer );
  Py_RETURN_NONE;

fail:
  PyBuffer_Release( &pyBuffer );
  return NULL;
}

/** An internal function that converts a python byte array
 * into a the image buffer with less copy operations.
 * The conversion is based on itk::ImportImageFilter.
 */
static PyObject*
sitk_SetImageViewFromArray( PyObject *SWIGUNUSEDPARM(self), PyObject *args )
{
  PyObject * pyImage = NULL;

  const void *buffer;
  Py_ssize_t buffer_len;
  Py_buffer  pyBuffer;
  memset(&pyBuffer, 0, sizeof(Py_buffer));

  sitk::Image * sitkImage = NULL;

  unsigned int dimension = 0;

  int ret = 0;


  // We wish to support both the new PEP3118 buffer interface and the
  // older. So we first try to parse the arguments with the new buffer
  // protocol, then the old.
  if (!PyArg_ParseTuple( args, "s*O", &pyBuffer, &pyImage ) )
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
    if( !PyArg_ParseTuple( args, "s#O", &buffer, &_len, &pyImage ) )
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

  try
    {
    switch( sitkImage->GetPixelIDValue() )
      {
      case sitk::sitkUnknown:
        PyErr_SetString( PyExc_RuntimeError, "Unknown pixel type." );
        goto fail;
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt8 != sitk::sitkUnknown, sitk::sitkVectorUInt8, -14 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<uint8_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<uint8_t, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkUInt8 != sitk::sitkUnknown, sitk::sitkUInt8, -2 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<uint8_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<uint8_t, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt8 != sitk::sitkUnknown, sitk::sitkVectorInt8, -15 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<int8_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<int8_t, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkInt8 != sitk::sitkUnknown, sitk::sitkInt8, -3 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<int8_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<int8_t, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt16 != sitk::sitkUnknown, sitk::sitkVectorUInt16, -16 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<uint16_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<uint16_t, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkUInt16 != sitk::sitkUnknown, sitk::sitkUInt16, -4 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<uint16_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<uint16_t, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt16 != sitk::sitkUnknown, sitk::sitkVectorInt16, -17 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<int16_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<int16_t, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkInt16 != sitk::sitkUnknown, sitk::sitkInt16, -5 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<int16_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<int16_t, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt32 != sitk::sitkUnknown, sitk::sitkVectorUInt32, -18 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<uint32_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<uint32_t, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkUInt32 != sitk::sitkUnknown, sitk::sitkUInt32, -6 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<uint32_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<uint32_t, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt32 != sitk::sitkUnknown, sitk::sitkVectorInt32, -19 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<int32_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<int32_t, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkInt32 != sitk::sitkUnknown, sitk::sitkInt32, -7 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<int32_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<int32_t, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorUInt64 != sitk::sitkUnknown, sitk::sitkVectorUInt64, -20 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<uint64_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<uint64_t, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkUInt64 != sitk::sitkUnknown, sitk::sitkUInt64, -8 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<uint64_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<uint64_t, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorInt64 != sitk::sitkUnknown, sitk::sitkVectorInt64, -21 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<int64_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<int64_t, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkInt64 != sitk::sitkUnknown, sitk::sitkInt64, -9 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<int64_t, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<int64_t, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorFloat32 != sitk::sitkUnknown, sitk::sitkVectorFloat32, -22 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<float, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<float, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkFloat32 != sitk::sitkUnknown, sitk::sitkFloat32, -10 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<float, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<float, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
        }
        break;
      case sitk::ConditionalValue< sitk::sitkVectorFloat64 != sitk::sitkUnknown, sitk::sitkVectorFloat64, -23 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportVectorImageBuffer<double, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
          {
          ret = sitkImportVectorImageBuffer<double, 3>(sitkImage, buffer, buffer_len);
          }
        else
          {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
          }
        break;
      case sitk::ConditionalValue< sitk::sitkFloat64 != sitk::sitkUnknown, sitk::sitkFloat64, -11 >::Value:
        if(dimension == 2)
          {
          ret = sitkImportScalarImageBuffer<double, 2>(sitkImage, buffer, buffer_len);
          }
        else if (dimension == 3)
        {
          ret = sitkImportScalarImageBuffer<double, 3>(sitkImage, buffer, buffer_len);
        }
        else
        {
          PyErr_SetString( PyExc_RuntimeError, "Unknown image dimension." );
          goto fail;
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

  PyBuffer_Release( &pyBuffer );
  Py_RETURN_NONE;

fail:
  PyBuffer_Release( &pyBuffer );
  return NULL;
}

#ifdef __cplusplus
} // end extern "C"



#endif
