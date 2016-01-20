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



#include "os_include.h"

#include <GL/glew.h>

#include "defines.h"
#include "FireCube.h"


namespace
{
    struct VERTEX_ELEMENT
    {
        float   position[3];
        float   normal[3];
        float   texcoord[2];
    };


    struct LIGHT_DATA
    {
        float   LightDirection[4];
        float   DiffuseColor[4]; 
        float   AmbientColor[4]; 
        float   SpecularColor[4]; 
    };
};


extern char* readRGBimage(int &xsize, int &ysize, int &channels, const char* fname);



FireCube::FireCube() :   m_uiVertexBuffer(0), m_uiElementBuffer(0), m_uiVertexArray(0), m_uiTexture(0), m_nSampler(0),
                             m_uiLightBuffer(0)
{
	m_program = NULL;

}


FireCube::~FireCube()
{
    if (m_uiVertexBuffer)
    {
        glDeleteBuffers(1, &m_uiVertexBuffer);
    }

    if (m_uiElementBuffer)
    {
        glDeleteBuffers(1, &m_uiElementBuffer);
    }

    if (m_uiLightBuffer)
    {
        glDeleteBuffers(1, &m_uiLightBuffer);
    }

    if (m_uiVertexArray)
    {
        glDeleteVertexArrays(1, &m_uiVertexArray);
    }

    if (m_uiTexture)
    {
        glDeleteTextures(1, &m_uiTexture);
    }
}







bool FireCube::initProgram()
{
    const char* pVShader = "   #version 420 compatibility \n\
                                           \n\
                                            layout(location = 0) in vec4 inVertex;\n\
                                            layout(location = 1) in vec4 inNormal;\n\
                                            layout(location = 2) in vec2 inTexCoord;\n\
											\n\
                                            out vec3 vNormal;\n\
                                            out vec3 vPosition;\n\
                                            out vec2 vTextureCoord;\n\
											\n\
                                            void main()\n\
                                            {\n\
                                                // transform vertex position into eye space\n\
                                                vPosition =  (gl_ModelViewMatrix * inVertex).xyz;\n\
												\n\
	                                            // Transform normal into eye space. Make sure last component is 0 to ignore translation in MV\n\
	                                            vNormal   = (gl_ModelViewMatrix * vec4(inNormal.x, inNormal.y, inNormal.z, 0.0f)).xyz;\n\
												\n\
	                                            vTextureCoord = inTexCoord;\n\
												\n\
	                                            gl_Position = gl_ModelViewProjectionMatrix * inVertex;\n\
                                            }\n\
											";
                                       

    const char* pFShader = "   #version 420 compatibility \n\
						   \n\
                                            layout(shared, binding = 1) uniform LightColor\n\
                                            {\n\
                                               vec4		LightDir;\n\
                                               vec4		DiffuseColor;\n\
                                               vec4		AmbientColor;\n\
                                               vec4		SpecularColor;\n\
                                            };\n\
											\n\
                                            layout(binding = 1) uniform sampler2D  BaseMap;\n\
											\n\
                                            in vec2 vTextureCoord;\n\
                                            in vec3 vNormal;\n\
                                            in vec3 vPosition;\n\
											\n\
											\n\
                                            void main()\n\
                                            {\n\
                                               vec4 vTexColor  = texture2D(BaseMap, vTextureCoord);\n\
                                               vec4 vBaseColor = mix(vec4(0.4f, 0.4f, 0.4f, 1.0f), vTexColor, vTexColor.a); \n\
											  \n\
                                               vec3 vN = normalize(vNormal);\n\
                                               \n\
                                               float NdotL = max(dot(vN, -LightDir.xyz), 0.0);\n\
											  \n\
                                               \n\
                                               vec3 vR    = reflect(LightDir.xyz, vN);  \n\
                                               float Spec = pow(max(dot(normalize(vR), normalize(-vPosition)), 0.0), 6.0);\n\
                                               \n\
                                               gl_FragColor = vBaseColor * DiffuseColor * NdotL + vBaseColor * AmbientColor + Spec * SpecularColor;\n\
											   \n\
                                            }\n\
                                         ";






	const char* vv = (const char*)pVShader;
	const char* ff = (const char*)pFShader;

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);	

	glShaderSource(vertexShader, 1, &vv,    NULL);
	glShaderSource(fragmentShader, 1, &ff,  NULL);

	glCompileShader(vertexShader);
	glCompileShader(fragmentShader);

	m_program = glCreateProgram();
	glAttachShader(m_program,vertexShader);
	glAttachShader(m_program,fragmentShader);
	glLinkProgram(m_program);


    return true;
}


bool FireCube::initTexture(const char* pFileName)
{
    int nTexWidth    = 0;
    int nTexHeight   = 0;
    int nTexChannels = 0;

    char* pPixels = readRGBimage(nTexWidth, nTexHeight, nTexChannels, pFileName);

    if (!pPixels)
    {
        return false;
    }

    glGenTextures(1, &m_uiTexture);

    glBindTexture(GL_TEXTURE_2D, m_uiTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, nTexWidth, nTexHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pPixels);

    glBindTexture(GL_TEXTURE_2D, 0);

    delete [] pPixels;

    return true;
}


bool FireCube::initBuffer()
{
    const VERTEX_ELEMENT vertexBuffer[] = { -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       0.0f, 0.0f,      // Front Tris (0,1,2) (0,2,3)
                                             0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       1.0f, 0.0f,
                                             0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       1.0f, 1.0f,
                                            -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,       0.0f, 1.0f,

                                             0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,       0.0f, 0.0f,      // Right Tris 4,5,6 4,6,7
                                             0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
                                             0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
                                             0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,       0.0f, 1.0f,

                                             0.5f, -0.5f, -0.5f,    0.0f, 0.0f,-1.0f,       0.0f, 0.0f,      // Back Tris 8,9,10 8,10,11
                                            -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,-1.0f,       1.0f, 0.0f,
                                            -0.5f,  0.5f, -0.5f,    0.0f, 0.0f,-1.0f,       1.0f, 1.0f,
                                             0.5f,  0.5f, -0.5f,    0.0f, 0.0f,-1.0f,       0.0f, 1.0f,

                                            -0.5f, -0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,       0.0f, 0.0f,      // Left Tris 12,13,14 12,14,15
                                            -0.5f, -0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,       1.0f, 0.0f,
                                            -0.5f,  0.5f,  0.5f,   -1.0f, 0.0f, 0.0f,       1.0f, 1.0f,
                                            -0.5f,  0.5f, -0.5f,   -1.0f, 0.0f, 0.0f,       0.0f, 1.0f,

                                            -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,       0.0f, 0.0f,      // Top Tris 16,17,18 16,18,19
                                             0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,       1.0f, 0.0f,
                                             0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f,       1.0f, 1.0f,
                                            -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f,       0.0f, 1.0f,

                                            -0.5f, -0.5f, -0.5f,    0.0f,-1.0f, 0.0f,       0.0f, 0.0f,      // Bottom Tris 20,21,22 20,22,23
                                             0.5f, -0.5f, -0.5f,    0.0f,-1.0f, 0.0f,       1.0f, 0.0f,
                                             0.5f, -0.5f,  0.5f,    0.0f,-1.0f, 0.0f,       1.0f, 1.0f,
                                            -0.5f, -0.5f,  0.5f,    0.0f,-1.0f, 0.0f,       0.0f, 1.0f };



    const GLushort indexBuffer[] = {  0, 1,  2,    0,  2,  3,           // Front
                                      4, 5,  6,    4,  6,  7,           // Right
                                      8, 9, 10,    8, 10, 11,           // Back
                                     12, 13, 14,  12, 14, 15,           // Left
                                     16, 17, 18,  16, 18, 19,           // Top
                                     20, 21, 22,  20, 22, 23  };        // Bottom

    // Create vertex buffer
    glGenBuffers(1, &m_uiVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVertexBuffer);

    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(VERTEX_ELEMENT), vertexBuffer, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Create element buffer
    glGenBuffers(1, &m_uiElementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiElementBuffer);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(GLushort), indexBuffer, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
 
    // Create UBO for light data
    glGenBuffers(1, &m_uiLightBuffer);
 
    glBindBuffer(GL_UNIFORM_BUFFER, m_uiLightBuffer);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(LIGHT_DATA), NULL, GL_DYNAMIC_DRAW);

    LIGHT_DATA* ptr = (LIGHT_DATA*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(LIGHT_DATA), GL_MAP_WRITE_BIT);

    if (!ptr)
    {
        return false;
    }

    ptr->LightDirection[0] =  0.0f;
    ptr->LightDirection[1] = -0.707107f; // 1 / sqrt(2.0)
    ptr->LightDirection[2] = -0.707107f; // 1 / sqrt(2.0)
    ptr->LightDirection[3] =  0.0f;

    ptr->AmbientColor[0] = 0.2f;
    ptr->AmbientColor[1] = 0.2f;
    ptr->AmbientColor[2] = 0.2f;
    ptr->AmbientColor[3] = 1.0f;

    ptr->DiffuseColor[0] = 1.0f;
    ptr->DiffuseColor[1] = 1.0f;
    ptr->DiffuseColor[2] = 1.0f;
    ptr->DiffuseColor[3] = 1.0f;

    ptr->SpecularColor[0] = 1.0f; 
    ptr->SpecularColor[1] = 1.0f;
    ptr->SpecularColor[2] = 1.0f;
    ptr->SpecularColor[3] = 1.0f;

    glUnmapBuffer(GL_UNIFORM_BUFFER);

    glBindBuffer(GL_UNIFORM_BUFFER,  0);

    return true;
}


bool FireCube::initVertexArray()
{
    // Create vertex arrays
    glGenVertexArrays(1, &m_uiVertexArray);
    glBindVertexArray(m_uiVertexArray);
    
    glBindBuffer(GL_ARRAY_BUFFER, m_uiVertexBuffer);

    // Enable vertex array to pass vertex data to the shader. 
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX_ELEMENT), 0); 


    // Enable vertex array to pass normal data to the shader.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, sizeof(VERTEX_ELEMENT),    ((char*)NULL + (3) * sizeof(float))   );

    // Enable vertex array to pass texture data to the shader. 
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VERTEX_ELEMENT),   ((char*)NULL + (6) * sizeof(float))    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_uiElementBuffer);

    glBindVertexArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return true;
}




bool FireCube::init()
{
	char textureFileName[] = "AMD_FirePro.rgb";

	if (!initTexture(textureFileName))
	{
		#if defined(_WIN32)
		MessageBox(NULL,"ERROR: can't open texture file.","ERROR",NULL);
		#else
		printf("ERROR: can't open file %s\n",textureFileName);
		#endif

        return false;
	}

    if (!initProgram())
        return false;

    if (!initBuffer())
        return false;

    if (!initVertexArray())
        return false;

    return true;
}




void FireCube::draw()
{    
    glUseProgram(m_program);



    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_uiTexture);

    glBindVertexArray(m_uiVertexArray);

    // Bind UBO containing light data to binding point 1
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_uiLightBuffer);
    
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, NULL);
    
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_2D, 0);


    glUseProgram(0);
}


