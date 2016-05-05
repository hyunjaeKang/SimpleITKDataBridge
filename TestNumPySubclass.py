import numpy as np
import SimpleITK as sitk

def CreateSITKImage(width, height):
  temp = 0
  Img = sitk.Image(width,height, sitk.sitkInt64)
  for w  in range(0, width):
    for h in range(0, height):
      Img.SetPixel(w,h,  temp)
      temp = temp + 1
  return Img

class sitkndarray(np.ndarray):
  """ A customized NumPy.ndarray for SimpleITK Image."""
  bConverted = False

  def __del__(self):
    pass

  def SetConvertedFlag(self, converted = True):
    self.bConverted = converted

  def __setitem__(self, item, to):
    if self.bConverted == True:
      temp = np.array(self, copy = True)
      self.data = temp.data
      self.bConverted = False
    super(sitkndarray, self).__setitem__(item, to)


#----- Test Code ----------------------------------
sitkImage   = CreateSITKImage(3,3)
ndArray     = sitk.GetArrayFromImage(sitkImage, arrayview = True, writeable = True)
sitkNDArray = ndArray.view(sitkndarray)
sitkNDArray.SetConvertedFlag()

print "\n--- Step 00 :: Create sitkImage, ndArray,sitkNDArray "
print "\n- ndArray"
print type(ndArray)
print ndArray
print "\n- sitkNDArray"
print type(sitkNDArray)
print sitkNDArray

print "\n\n--- Step 01 :: Modify a pixel of sitkImage "
print "sitkImage.SetPixel((0,0), -300)"
sitkImage.SetPixel((0,0), -300)
print "\n sitkImage.GetPixel(0,0) = ", sitkImage.GetPixel(0,0)
print "\n- ndArray"
print ndArray
print "\n- sitkNDArray"
print sitkNDArray

print "\n\n--- Step 02 :: Modify a pixel of ndArray "
print "ndArray[0,0] = 1000"
ndArray[0,0] = 1000
print "\n sitkImage.GetPixel(0,0) = ", sitkImage.GetPixel(0,0)
print "\n- ndArray"
print ndArray
print "\n- sitkNDArray"
print sitkNDArray

print "\n\n--- Step 03 :: Modify a pixel of sitkNDArray "
print "sitkNDArray[0,0] = -7000"
sitkNDArray[0,0] = -7000
print "\n sitkImage.GetPixel(0,0) = ", sitkImage.GetPixel(0,0)
print "\n- ndArray"
print ndArray
print "\n- sitkNDArray"
print sitkNDArray






#print "* Step 01"
#nparray = np.zeros((3,3), dtype = np.int32)
#nparray[0,0] =  100
#nparray[1,1] = -100
#print type(nparray)
#print nparray

#print "* Step 02"
#Snparray = nparray.view(sitkndarray)
#print type(Snparray)
#print Snparray

#print "* Step 03"
#Snparray[0,0] = -300
#print Snparray
#print nparray




#arrayView = numpy.asarray(imageMemoryView).view(dtype = dtype).reshape(shape[::-1])
#Snparray = np.zeros((3,3), dtype = np.int32).view(sitkndarray)
#print type(Snparray)
#help(Snparray)
#print Snparray[0,0]
#Snparray[0,0] = 10
#print Snparray[0,0]
