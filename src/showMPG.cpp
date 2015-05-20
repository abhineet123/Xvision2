#include <XVMpeg.h>
#include <XVWindowX.h>
#include <XVVideo.h>
#include "XVImageBase.h"
#include <XVImageScalar.h>

int main(int argc, char * argv[]){
	typedef XVMpeg<XVImageRGB<XV_RGB> >  MPG;
	typedef XVVideo<XVImageRGB<XV_RGB> > VID;
	VID *vid = new MPG(argv[1]);

}
