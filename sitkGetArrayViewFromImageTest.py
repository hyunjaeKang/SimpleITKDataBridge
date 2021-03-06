#==========================================================================
#
#   Copyright Insight Software Consortium
#
#   Licensed under the Apache License, Version 2.0 (the "License");
#   you may not use this file except in compliance with the License.
#   You may obtain a copy of the License at
#
#          http://www.apache.org/licenses/LICENSE-2.0.txt
#
#   Unless required by applicable law or agreed to in writing, software
#   distributed under the License is distributed on an "AS IS" BASIS,
#   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#   See the License for the specific language governing permissions and
#   limitations under the License.
#
#==========================================================================*/
from __future__ import print_function
import sys
import unittest
import datetime as dt


import SimpleITK as sitk
import numpy as np

sizeX = 4
sizeY = 5
sizeZ = 3
newSimpleITKPixelValueInt32 = -3000
newNumPyElementValueInt32   = 200



class TestNumpySimpleITKMemoryviewInterface(unittest.TestCase):
    """ This tests numpy array <-> SimpleITK Image conversion. """

    def setUp(self):
        pass

    def _helper_check_sitk_to_numpy_type(self, sitkType, numpyType):
        image = sitk.Image( (9, 10), sitkType, 1 )
        a = sitk.GetArrayFromImage( image, arrayview = True)
        self.assertEqual( numpyType, a.dtype )
        self.assertEqual( (10, 9), a.shape )

    def test_type_to_numpy(self):
        "try all sitk pixel type to convert to NumPy array view"

        self._helper_check_sitk_to_numpy_type(sitk.sitkUInt8, np.uint8)
        self._helper_check_sitk_to_numpy_type(sitk.sitkUInt16, np.uint16)
        self._helper_check_sitk_to_numpy_type(sitk.sitkUInt32, np.uint32)
        if sitk.sitkUInt64 != sitk.sitkUnknown:
            self._helper_check_sitk_to_numpy_type(sitk.sitkUInt64, np.uint64)
        self._helper_check_sitk_to_numpy_type(sitk.sitkInt8, np.int8)
        self._helper_check_sitk_to_numpy_type(sitk.sitkInt16, np.int16)
        self._helper_check_sitk_to_numpy_type(sitk.sitkInt32, np.int32)
        if sitk.sitkInt64 != sitk.sitkUnknown:
            self._helper_check_sitk_to_numpy_type(sitk.sitkInt64, np.int64)
        self._helper_check_sitk_to_numpy_type(sitk.sitkFloat32, np.float32)
        self._helper_check_sitk_to_numpy_type(sitk.sitkFloat64, np.float64)
        self._helper_check_sitk_to_numpy_type(sitk.sitkVectorUInt8, np.uint8)
        self._helper_check_sitk_to_numpy_type(sitk.sitkVectorInt8, np.int8)
        self._helper_check_sitk_to_numpy_type(sitk.sitkVectorUInt16, np.uint16)
        self._helper_check_sitk_to_numpy_type(sitk.sitkVectorInt16, np.int16)
        self._helper_check_sitk_to_numpy_type(sitk.sitkVectorUInt32, np.uint32)
        self._helper_check_sitk_to_numpy_type(sitk.sitkVectorInt32, np.int32)
        if sitk.sitkVectorUInt64 != sitk.sitkUnknown:
            self._helper_check_sitk_to_numpy_type(sitk.sitkVectorUInt64, np.uint64)
        if sitk.sitkVectorInt64 != sitk.sitkUnknown:
            self._helper_check_sitk_to_numpy_type(sitk.sitkVectorInt64, np.int64)
        self._helper_check_sitk_to_numpy_type(sitk.sitkVectorFloat32, np.float32)
        self._helper_check_sitk_to_numpy_type(sitk.sitkVectorFloat64, np.float64)

    def test_to_numpy_and_back(self):
        """Test converting an image to NumPy array view and back"""

        img = sitk.GaussianSource( sitk.sitkFloat32,  [100,100], sigma=[10]*3, mean = [50,50] )

        h = sitk.Hash( img )

        img2 = sitk.GetImageFromArray( sitk.GetArrayFromImage( img, arrayview = True), imageview = True)
        self.assertEqual( h, sitk.Hash( img2 ))

    def test_vector_image_to_numpy(self):
        """Test converting back and forth between NumPy array view and SimpleITK
        images were the SimpleITK image has multiple componets and
        stored as a VectorImage."""


        # Check 2D
        img = sitk.PhysicalPointSource(sitk.sitkVectorFloat32, [3,4])
        h = sitk.Hash( img )

        nda = sitk.GetArrayFromImage(img, arrayview = True)

        self.assertEqual(nda.shape, (4,3,2))
        self.assertEqual(nda[0,0].tolist(), [0,0])
        self.assertEqual(nda[2,1].tolist(), [1,2])
        self.assertEqual(nda[0,:,0].tolist(), [0,1,2])

        img2 = sitk.GetImageFromArray( nda, isVector=True, imageview = True)
        self.assertEqual( h, sitk.Hash(img2) )

        # check 3D
        img = sitk.PhysicalPointSource(sitk.sitkVectorFloat32, [3,4,5])
        h = sitk.Hash( img )

        nda = sitk.GetArrayFromImage(img, arrayview = True)

        self.assertEqual(nda.shape, (5,4,3,3))
        self.assertEqual(nda[0,0,0].tolist(), [0,0,0])
        self.assertEqual(nda[0,0,:,0].tolist(), [0,1,2])
        self.assertEqual(nda[0,:,1,1].tolist(), [0,1,2,3])


        img2 = sitk.GetImageFromArray(nda, imageview = True)
        self.assertEqual(img2.GetSize(), img.GetSize())
        self.assertEqual(img2.GetNumberOfComponentsPerPixel(), img.GetNumberOfComponentsPerPixel())
        self.assertEqual(h, sitk.Hash(img2))


    def test_legacy(self):
      """Test SimpleITK Image to numpy memoryview."""

      # 2D image
      image = sitk.Image(sizeX, sizeY, sitk.sitkInt32)
      for j in range(sizeY):
          for i in range(sizeX):
              image[i, j] = j*sizeX + i

      print(sitk.GetArrayFromImage(image, arrayview = True))

      self.assertEqual( type (sitk.GetArrayFromImage(image, arrayview = True)),  np.ndarray )

      # 3D image
      image = sitk.Image(sizeX, sizeY, sizeZ, sitk.sitkFloat32)
      for k in range(sizeZ):
          for j in range(sizeY):
              for i in range(sizeX):
                  image[i, j, k] = (sizeY*k +j)*sizeX + i

      print(sitk.GetArrayFromImage(image, arrayview = True))

      self.assertEqual( type (sitk.GetArrayFromImage(image, arrayview = True)),  np.ndarray )

    def test_processing_time(self):
      """Check the processing time the conversions from SimpleITK Image
         to numpy array (GetArrayFromImage) and
         numpy memoryview (GetArrayViewFromImage)."""

      # Performance test for SimpleITK Image -> NumPy array
      img = sitk.GaussianSource( sitk.sitkFloat32,  [3000,3000], sigma=[10]*3, mean = [50,50] )

      print("\nGet NumPy array from 3000x3000 SimpleITK Image")
      nparray_time_start     =  dt.datetime.now()
      nparray                = sitk.GetArrayFromImage(img)
      nparray_time_elapsed   = dt.datetime.now() - nparray_time_start
      print ("Processing time of GetArrayFromImage (Copy operation)  :: %.1f (us)"%nparray_time_elapsed.microseconds)

      npview_time_start      = dt.datetime.now()
      npview                 = sitk.GetArrayFromImage(img, arrayview = True)
      npview_time_elapsed    = dt.datetime.now() - npview_time_start
      print ("Processing time of GetArrayFromImage (Array view)      :: %.1f (us)"%npview_time_elapsed.microseconds)

      self.assertTrue( nparray_time_elapsed > npview_time_elapsed)

      # Performance test for NumPy array -> SimpleITK Image
      Big_nparray = np.zeros((3000,3000), dtype = np.int64);

      print("\nGet SimpleITK Image from 3000x3000 NumPy array")
      time_start             = dt.datetime.now()
      img_copy               = sitk.GetImageFromArray(Big_nparray)
      time_elapsed_copy      = dt.datetime.now() - time_start
      print ("Processing time of GetImageFromArray (Copy operation)  :: %.1f (us)"%time_elapsed_copy.microseconds)

      time_start             =  dt.datetime.now()
      imgview                = sitk.GetImageFromArray(Big_nparray, imageview = True)
      time_elapsed_imageview = dt.datetime.now() - time_start
      print ("Processing time of GetImageFromArray (Image view)      :: %.1f (us)"%time_elapsed_imageview.microseconds)

      self.assertTrue( time_elapsed_copy > time_elapsed_imageview)

    def test_numpy_memoryview_writeable_flag(self):
      """Test the second argument (writeable) of the GetArrayViewFromImage function."""

      image = sitk.Image(sizeX, sizeY, sitk.sitkInt32)
      for j in range(sizeY):
          for i in range(sizeX):
              image[i, j] = j*sizeX + i

      npviewReadOnly  = sitk.GetArrayFromImage(image, arrayview = True, writeable = False)
      npviewReadWrite = sitk.GetArrayFromImage(image, arrayview = True, writeable = True)

      image.SetPixel(0,0, newSimpleITKPixelValueInt32)

      self.assertEqual( npviewReadOnly[0][0],newSimpleITKPixelValueInt32)
      self.assertEqual( npviewReadWrite[0][0],newSimpleITKPixelValueInt32)

      try:
        npviewReadOnly[0][0] = newNumPyElementValueInt32
      except:
        print ("We cannot modify an element value of the NumPy array view", \
                "which was converted with ReadOnly mode.")

      npviewReadWrite[0][0] = newNumPyElementValueInt32

      self.assertEqual( image.GetPixel(0,0),newNumPyElementValueInt32)

    def test_NumPy_arrayview_deletion_sitkImage_1(self):
      # 2D image
      image = sitk.Image(sizeX, sizeY, sitk.sitkInt32)
      for j in range(sizeY):
          for i in range(sizeX):
              image[i, j] = j*sizeX + i

      npview = sitk.GetArrayFromImage(image, arrayview = True, writeable = True)
      image.SetPixel(0,0, newSimpleITKPixelValueInt32)

      del image

      carr = np.array(npview, copy = False)
      rarr = np.reshape(npview, (1, npview.size))
      varr = npview.view(dtype=np.int8)

      self.assertEqual( carr[0,0],newSimpleITKPixelValueInt32)
      self.assertEqual( rarr[0,0],newSimpleITKPixelValueInt32)
      self.assertEqual( varr[0,0],np.int8(newSimpleITKPixelValueInt32))

      npview[0,0] = newNumPyElementValueInt32

      del npview

      self.assertEqual( carr[0,0],newNumPyElementValueInt32)
      self.assertEqual( rarr[0,0],newNumPyElementValueInt32)
      self.assertEqual( varr[0,0],np.int8(newNumPyElementValueInt32))

    def test_NumPy_arrayview_deletion_sitkImage_2(self):
      # 2D image
      sizeX                                = 3
      sizeY                                = 3
      imgNewPixelValueInt32                = 30
      imgSitkShallowCopyNewPixelValueInt32 = -50

      img = sitk.Image(sizeX, sizeY, sitk.sitkInt32)
      for j in range(sizeY):
        for i in range(sizeX):
          img[i, j] = j*sizeX + i

      imgSitkShallowCopy = sitk.Image(img)

      arr = sitk.GetArrayFromImage(img, arrayview = True)

      # img and imgSitkShallCopy have different pixel buffers because of a deep copy
      img.SetPixel(0,0,imgNewPixelValueInt32)
      imgSitkShallowCopy.SetPixel(0,0,imgSitkShallowCopyNewPixelValueInt32)

      del imgSitkShallowCopy

      self.assertEqual( arr[0,0],imgNewPixelValueInt32)

    def test_sitkImageView_deletions_1(self):
      npArrayPixelValueInt32                = 30
      nparray = np.zeros((3,3), dtype = np.int32);
      nparray[0,0] = npArrayPixelValueInt32

      img = sitk.GetImageFromArray(nparray, imageview = True)

      del nparray

      self.assertEqual(img.GetPixel(0,0), npArrayPixelValueInt32)

if __name__ == '__main__':
    unittest.main()
