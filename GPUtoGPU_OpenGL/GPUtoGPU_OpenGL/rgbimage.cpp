//--------------------------------------------------------------------------------------
//
//
// Copyright 2014 ADVANCED MICRO DEVICES, INC.  All Rights Reserved.
//
// AMD is granting you permission to use this software and documentation (if
// any) (collectively, the "Materials") pursuant to the terms and conditions
// of the Software License Agreement included with the Materials.  If you do
// not have a copy of the Software License Agreement, contact your AMD
// representative for a copy.
// You agree that you will not reverse engineer or decompile the Materials,
// in whole or in part, except as allowed by applicable law.
//
// WARRANTY DISCLAIMER: THE SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND.  AMD DISCLAIMS ALL WARRANTIES, EXPRESS, IMPLIED, OR STATUTORY,
// INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE, NON-INFRINGEMENT, THAT THE SOFTWARE
// WILL RUN UNINTERRUPTED OR ERROR-FREE OR WARRANTIES ARISING FROM CUSTOM OF
// TRADE OR COURSE OF USAGE.  THE ENTIRE RISK ASSOCIATED WITH THE USE OF THE
// SOFTWARE IS ASSUMED BY YOU.
// Some jurisdictions do not allow the exclusion of implied warranties, so
// the above exclusion may not apply to You.
//
// LIMITATION OF LIABILITY AND INDEMNIFICATION:  AMD AND ITS LICENSORS WILL
// NOT, UNDER ANY CIRCUMSTANCES BE LIABLE TO YOU FOR ANY PUNITIVE, DIRECT,
// INCIDENTAL, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES ARISING FROM USE OF
// THE SOFTWARE OR THIS AGREEMENT EVEN IF AMD AND ITS LICENSORS HAVE BEEN
// ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
// In no event shall AMD's total liability to You for all damages, losses,
// and causes of action (whether in contract, tort (including negligence) or
// otherwise) exceed the amount of $100 USD.  You agree to defend, indemnify
// and hold harmless AMD and its licensors, and any of their directors,
// officers, employees, affiliates or agents from and against any and all
// loss, damage, liability and other expenses (including reasonable attorneys'
// fees), resulting from Your use of the Software or violation of the terms and
// conditions of this Agreement. 
//
// U.S. GOVERNMENT RESTRICTED RIGHTS: The Materials are provided with "RESTRICTED
// RIGHTS." Use, duplication, or disclosure by the Government is subject to the
// restrictions as set forth in FAR 52.227-14 and DFAR252.227-7013, et seq., or
// its successor.  Use of the Materials by the Government constitutes
// acknowledgement of AMD's proprietary rights in them.
//
// EXPORT RESTRICTIONS: The Materials may be subject to export restrictions as
// stated in the Software License Agreement.
//
//--------------------------------------------------------------------------------------


//
//  Generates RGB image(SGI Image File Format) file for the specified data values 
//


#include <cstdio>
#include <stdlib.h> 
#include <string.h>
#include <fstream>

using namespace std;
 

static int putlong(ofstream &outf, unsigned long val);


static void putbyte(ofstream &outf, unsigned char val)
{
	unsigned char buf[1];  
	buf[0] = val;
	outf.write((const char*)buf, sizeof(buf));
}


void putshort(ofstream &outf, unsigned short val)  
{
	unsigned char buf[2];
     
	buf[0] = (val>>8);
	buf[1] = (val>>0);
	outf.write((const char*)buf, sizeof(buf));
}



static int putlong(ofstream &outf, unsigned long val)
{

	unsigned char buf[4];
     
	buf[0] = (unsigned char)(val>>24);
	buf[1] = (unsigned char)(val>>16);
	buf[2] = (unsigned char)(val>>8);
	buf[3] = (unsigned char)(val>>0);

	outf.write((const char*)buf, sizeof(buf));

	return ((int)outf.tellp());
}


static void getbyte(ifstream &inf, unsigned char* val)
{
    if (inf.is_open() && val)
    {
        inf.read((char *)val, 1);
    }
}


static void getshort(ifstream &inf, unsigned short* val)
{
    if (inf.is_open() && val)
    {
        unsigned char buf[2];

        inf.read((char *)buf, 2);

        (*val)  = (unsigned short)buf[0];
        (*val)  = (*val)<<8;
        (*val) |= (unsigned short)buf[1];
    }
}


static void getlong(ifstream &inf, unsigned long* val)
{
    if (inf.is_open() && val)
    {
        unsigned char buf[4];

        inf.read((char *)buf, 4);

        (*val)  = (unsigned short)buf[0];
        (*val)  = (*val)<<24;
        (*val) |= (unsigned short)buf[1];
        (*val)  = (*val)<<16;
        (*val)  = (unsigned short)buf[2];
        (*val)  = (*val)<<8;
        (*val) |= (unsigned short)buf[3];
    }
}



//  function to generate RGB file from the buffer data
extern void writeRGBimage(unsigned char* outbuf, int xsize, int ysize, int channels, const char* fname)
{
	char iname[80];
	int i, x, y;
  
	ofstream of ((const char*)fname, ios::out|ios::binary);
 
	if(!of) 
	{
	    fprintf(stderr,"sgiimage: can't open output file\n");
	    exit(1);
	}

	putshort(of, 474);       //  MAGIC  

	putbyte(of, 0);          //  STORAGE is VERBATIM 

	putbyte(of, 1);          //  BPC is 1            

	putshort(of, 2);         //  DIMENSION is 2      

	putshort(of, xsize);     //  XSIZE      

	putshort(of, ysize);     //  YSIZE      

	putshort(of, channels);  //  Channels   

	putlong(of, 0);          //  PIXMIN is 0   

	putlong(of, 255);        //  PIXMAX is 255  

	for(i = 0; i < 4; i++)   //   DUMMY 4 bytes       
	    putbyte(of,0);
	

	#if defined( WIN32 )
	strcpy_s
	#else
	strcpy
	#endif
	(iname,"No Name");


	of.write((const char*)iname, sizeof(iname));   // IMAGENAME  

	putlong(of, 0);            //  COLORMAP is 0   

	for(i = 0; i < 404; i++)   //  DUMMY 404 bytes     
	    putbyte(of, 0);

	for(int z = 0; z < channels;z++) 
	{
		for(y = 0; y < ysize; y++) 
		{
			for(x = 0; x < xsize; x++) 
			{
				of.put((char)outbuf[(((channels * y * xsize) + channels * x)) + z]);
				
			}
		}
	}

	of.close();

}



char* readRGBimage(int &xsize, int &ysize, int &channels, const char* fname)
{
    ifstream imageFile;

    imageFile.open(fname, ios::in | ios::binary);

    if (!imageFile.is_open())
    {
        return NULL;
    }

    ///////////////////////////////////////////////////////////////////////////////
    // Read Header
    //
    //    2 bytes | short  | MAGIC     | IRIS image file magic number
    //    1 byte  | char   | STORAGE   | Storage format
    //    1 byte  | char   | BPC       | Number of bytes per pixel channel
    //    2 bytes | ushort | DIMENSION | Number of dimensions
    //    2 bytes | ushort | XSIZE     | X size in pixels
    //    2 bytes | ushort | YSIZE     | Y size in pixels
    //    2 bytes | ushort | ZSIZE     | Number of channels
    //    4 bytes | long   | PIXMIN    | Minimum pixel value
    //    4 bytes | long   | PIXMAX    | Maximum pixel value
    //    4 bytes | char   | DUMMY     | Ignored
    //   80 bytes | char   | IMAGENAME | Image name
    //    4 bytes | long   | COLORMAP  | Colormap ID
    //  404 bytes | char   | DUMMY     | Ignored
    //
    ///////////////////////////////////////////////////////////////////////////////
    unsigned char  ucBuffer;
    unsigned short usBuffer;
    unsigned long  ulBuffer;

    getshort(imageFile, &usBuffer);    
    if (usBuffer != 474)
        return NULL;

    getbyte(imageFile,  &ucBuffer); 
    if (ucBuffer != 0)
        return NULL;

    getbyte(imageFile,  &ucBuffer);  
    if (ucBuffer != 1)
        return NULL;

    getshort(imageFile, &usBuffer);
    if (ucBuffer != 1)
        return NULL;

    getshort(imageFile, &usBuffer);
    xsize = usBuffer;

    getshort(imageFile, &usBuffer);
    ysize = usBuffer;

    getshort(imageFile, &usBuffer);
    channels = usBuffer;

    getlong(imageFile, &ulBuffer);
    if (ulBuffer != 0)
        return NULL;

    getlong(imageFile, &ulBuffer);
    if (ulBuffer != 255)
        return NULL;

    // Ignore the next 492 bytes
    for (unsigned int i = 0; i < 492; ++i)
    {
        getbyte(imageFile, &ucBuffer);
    }

    char* pPixels = new char[xsize*ysize*channels];

    for (int z = 0; z < channels; ++z)
    {
        for (int y = 0; y < ysize; ++y)
        {
            for (int x = 0; x < xsize; ++x)
            {
                char *pPtr = &(pPixels[y*xsize*channels + x*channels + z]);

                getbyte(imageFile, (unsigned char*)pPtr);
            }
        }
    }
   
    return pPixels;
}