/*
*/

#include "base.h"
#include "GLItemViewer.h"

//-----------------------------------------------------------------------------
// TEMP
extern e_ItemType eType;

extern int                m_nFC;
extern __VertexXyzNormal* m_pVertices;
extern uint16_t*              m_pwVtxIndices;
extern float*             m_pfUVs;
extern uint16_t*              m_pwUVsIndices;

extern _N3TexHeader    HeaderOrg;
extern uint8_t*  compTexData;
extern int             compTexSize;
extern int             iPixelSize;
extern uint16_t* m_pIndices0;
extern _N3VertexT1*    m_pVertices0;
extern int             m_iMaxNumIndices0;
extern int             m_iMaxNumVertices0;

//-----------------------------------------------------------------------------
GLItemViewer::GLItemViewer(int x, int y, int w, int h, const char* l):
Fl_Gl_Window(x, y, w, h, l) {
	min_y = FLT_MAX;
	max_x = max_y = max_z = FLT_MIN;
	bShadersBuilt = false;
	Fl::add_timeout(1.0/24.0, RenderCallBack, (void*)this);
	end();
}

//-----------------------------------------------------------------------------
bool GLItemViewer::BuildShaders(void) {
	if(bShadersBuilt) return true;

	GLenum glError = glewInit();
	if(glError != GLEW_OK) {
		fprintf(stderr, "ERROR(glewInit): %s\n", glewGetErrorString(glError));
		return false;
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	const GLchar* vertSource = {
		"#version 150 core\n"\
		"\n"\
		"in vec3 pos;\n"\
		"in vec2 texcoord;\n"\
		"\n"\
		"out vec2 fragTexcoord;\n"\
		"\n"\
		"uniform mat4 model;\n"\
		"uniform mat4 view;\n"\
		"uniform mat4 proj;\n"\
		"\n"\
		"void main() {\n"\
			"fragTexcoord = texcoord;\n"\
			"gl_Position = proj*view*model*vec4(pos, 1.0);\n"\
		"}\n"
	};

	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, &vertSource, nullptr);
	glCompileShader(vertShader);

	GLint status;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE) {
		char buffer[512];
		glGetShaderInfoLog(vertShader, 512, nullptr, buffer);
		fprintf(stderr, "ERROR(glCompileShader):\n%s\n", buffer);
		return false;
	}

	const GLchar* fragSource = {
		"#version 150 core\n"\
		"\n"\
		"in vec2 fragTexcoord;\n"\
		"uniform sampler2D tex;\n"\
		"\n"\
		"void main() {\n"\
			"gl_FragColor = texture2D(tex, fragTexcoord)*vec4(1.0, 1.0, 1.0, 1.0);\n"\
		"}\n"
	};

	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, &fragSource, nullptr);
	glCompileShader(fragShader);

	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
	if(status == GL_FALSE) {
		char buffer[512];
		glGetShaderInfoLog(fragShader, 512, nullptr, buffer);
		fprintf(stderr, "ERROR(glCompileShader):\n%s\n", buffer);
		return false;
	}

	glProgram = glCreateProgram();
	glAttachShader(glProgram, vertShader);
	glAttachShader(glProgram, fragShader);
	glLinkProgram(glProgram);

	glGetProgramiv(glProgram, GL_LINK_STATUS, &status);
	if(status == GL_FALSE) {
		char buffer[512];
		glGetProgramInfoLog(glProgram, 512, nullptr, buffer);
		fprintf(stderr, "ERROR(glLinkProgram):\n%s\n", buffer);
		return false;
	}

	glUseProgram(glProgram);

	glUniModel = glGetUniformLocation(glProgram, "model");
	glUniView  = glGetUniformLocation(glProgram, "view");
	glUniProj  = glGetUniformLocation(glProgram, "proj");

	glGenVertexArrays(1, &glVertArray);
	glGenBuffers(1, &glVertBuffer);
	glGenBuffers(1, &glEleBuffer);
	glGenTextures(1, &glTeXBuffer);

	bShadersBuilt = true;

	return true;
}

//-----------------------------------------------------------------------------
void GLItemViewer::PushDataToGPU(void) {
	min_y = FLT_MAX;
	max_x = max_y = max_z = FLT_MIN;
	glBindVertexArray(glVertArray);

	if(eType == ITEM_TYPE_PLUG) {
		GLfloat* vertices = new GLfloat[5*m_iMaxNumVertices0];
		memset(vertices, 0, 5*m_iMaxNumVertices0);

		for(int i=0; i<m_iMaxNumVertices0; i++) {
			if(m_pVertices0[i].y < min_y)
				min_y = m_pVertices0[i].y;
			if(fabs(m_pVertices0[i].x) > max_x)
				max_x = fabs(m_pVertices0[i].x);
			if(m_pVertices0[i].y > max_y)
				max_y = m_pVertices0[i].y;
			if(fabs(m_pVertices0[i].z) > max_z)
				max_z = fabs(m_pVertices0[i].z);

			vertices[5*i+0] = m_pVertices0[i].x;
			vertices[5*i+1] = m_pVertices0[i].y;
			vertices[5*i+2] = m_pVertices0[i].z;

			vertices[5*i+3] = m_pVertices0[i].tu;
			vertices[5*i+4] = m_pVertices0[i].tv;
		}

		glBindBuffer(GL_ARRAY_BUFFER, glVertBuffer);
		glBufferData(GL_ARRAY_BUFFER, 5*m_iMaxNumVertices0*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
		delete vertices;

		GLint posAttrib = glGetAttribLocation(glProgram, "pos");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
		glEnableVertexAttribArray(posAttrib);
		GLint texAttrib = glGetAttribLocation(glProgram, "texcoord");
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
		glEnableVertexAttribArray(texAttrib);

		GLuint* elements = new GLuint[m_iMaxNumIndices0];
		memset(elements, 0, m_iMaxNumIndices0*sizeof(GLuint));
		for(int i=0; i<m_iMaxNumIndices0; i++) {
			elements[i] = (GLuint) m_pIndices0[i];
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEleBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_iMaxNumIndices0*sizeof(GLuint), elements, GL_STATIC_DRAW);
		delete elements;

	} else if(eType == ITEM_TYPE_PART) {
		GLfloat* vertices = new GLfloat[5*3*m_nFC];
		memset(vertices, 0, 5*3*m_nFC);

		for(int i=0; i<3*m_nFC; i++) {
			if(m_pVertices[m_pwVtxIndices[i]].y < min_y)
				min_y = m_pVertices[m_pwVtxIndices[i]].y;
			if(fabs(m_pVertices[m_pwVtxIndices[i]].x) > max_x)
				max_x = fabs(m_pVertices[m_pwVtxIndices[i]].x);
			if(m_pVertices[m_pwVtxIndices[i]].y > max_y)
				max_y = m_pVertices[m_pwVtxIndices[i]].y;
			if(fabs(m_pVertices[m_pwVtxIndices[i]].z) > max_z)
				max_z = fabs(m_pVertices[m_pwVtxIndices[i]].z);

			vertices[5*i+0] = m_pVertices[m_pwVtxIndices[i]].x;
			vertices[5*i+1] = m_pVertices[m_pwVtxIndices[i]].y;
			vertices[5*i+2] = m_pVertices[m_pwVtxIndices[i]].z;

			vertices[5*i+3] = m_pfUVs[2*m_pwUVsIndices[i]+0];
			vertices[5*i+4] = m_pfUVs[2*m_pwUVsIndices[i]+1];
		}

		glBindBuffer(GL_ARRAY_BUFFER, glVertBuffer);
		glBufferData(GL_ARRAY_BUFFER, 5*3*m_nFC*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
		delete vertices;

		GLint posAttrib = glGetAttribLocation(glProgram, "pos");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
		glEnableVertexAttribArray(posAttrib);
		GLint texAttrib = glGetAttribLocation(glProgram, "texcoord");
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
		glEnableVertexAttribArray(texAttrib);

		GLuint* elements = new GLuint[3*m_nFC];
		memset(elements, 0, 3*m_nFC*sizeof(GLuint));
		for(int i=0; i<3*m_nFC; i++) {
			elements[i] = (GLuint) (i);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEleBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3*m_nFC*sizeof(GLuint), elements, GL_STATIC_DRAW);
		delete elements;

	} else if(eType==ITEM_TYPE_ICONONLY || eType==ITEM_TYPE_GOLD || eType==ITEM_TYPE_SONGPYUN) {
		// NOTE: need to offset the position to get the icon to display in the center
		float vertices[] = {
			-0.25f+0.25f/4.0f,  0.25f-0.25f/4.0f, -0.1f, 0.0f, 0.0f, // Top-left
			 0.25f+0.25f/4.0f,  0.25f-0.25f/4.0f, -0.1f, 1.0f, 0.0f, // Top-right
			 0.25f+0.25f/4.0f, -0.25f-0.25f/4.0f, -0.1f, 1.0f, 1.0f, // Bottom-right
			-0.25f+0.25f/4.0f, -0.25f-0.25f/4.0f, -0.1f, 0.0f, 1.0f  // Bottom-left
		};

		glBindBuffer(GL_ARRAY_BUFFER, glVertBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		GLint posAttrib = glGetAttribLocation(glProgram, "pos");
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), 0);
		glEnableVertexAttribArray(posAttrib);
		GLint texAttrib = glGetAttribLocation(glProgram, "texcoord");
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 5*sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));
		glEnableVertexAttribArray(texAttrib);

		GLuint elements[] = {
			0, 1, 2,
			2, 3, 0
		};

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glEleBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glTeXBuffer);
	
	GLenum texType;
	GLenum texFormat;
	switch(HeaderOrg.Format) {
		case D3DFMT_DXT1: {
			texFormat = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		} break;
		case D3DFMT_DXT3: {
			texFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		} break;
		case D3DFMT_DXT5: {
			texFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		} break;
		case D3DFMT_A1R5G5B5: {
			texFormat = GL_RGBA;
			texType = GL_UNSIGNED_SHORT_5_5_5_1;
			for(int i=0; i<HeaderOrg.nWidth*HeaderOrg.nHeight; ++i) {
				uint16_t* pp = (uint16_t*)(compTexData + iPixelSize*i);
				uint16_t p = *pp;
				uint16_t np = ((p&0x7C00)>>10)<<11|((p&0x3E0)>>5)<<6|(p&0x1F)<<1|((p&0x8000)>>15);
				*pp = np;
			}
		} break;
		case D3DFMT_A4R4G4B4: {
			texFormat = GL_RGBA;
			texType = GL_UNSIGNED_SHORT_4_4_4_4;
			for(int i=0; i<HeaderOrg.nWidth*HeaderOrg.nHeight; ++i) {
				uint16_t* pp = (uint16_t*)(compTexData + iPixelSize*i);
				uint16_t p = *pp;
				uint16_t np = ((p&0xF00)>>8)<<12|((p&0xF0)>>4)<<8|(p&0xF)<<4|((p&0xF000)>>12);
				*pp = np;
			}
		} break;
		case D3DFMT_R8G8B8: {
			texFormat = GL_RGB;
			texType = GL_UNSIGNED_BYTE;
			fprintf(stderr, "\nNeed to implement this D3DFMT_R8G8B8\n");
			return;
		} break;
		case D3DFMT_A8R8G8B8: {
			texFormat = GL_RGBA;
			texType = GL_UNSIGNED_INT_8_8_8_8;
			fprintf(stderr, "\nNeed to implement this D3DFMT_A8R8G8B8\n");
			return;
		} break;
		case D3DFMT_X8R8G8B8: {
			texFormat = GL_RGBA;
			texType = GL_UNSIGNED_INT_8_8_8_8;
			fprintf(stderr, "\nNeed to implement this D3DFMT_X8R8G8B8\n");
			return;
		} break;
		default: {
			fprintf(stderr,
				"\nERROR: Unknown texture format %d. (need to implement this)\n", HeaderOrg.Format
			);
			return;
		} break;
	}

	if(iPixelSize == 0) {
		glCompressedTexImage2D(GL_TEXTURE_2D, 0, texFormat, HeaderOrg.nWidth, HeaderOrg.nHeight, 0, compTexSize, compTexData);
	} else {
		glTexImage2D(GL_TEXTURE_2D, 0, texFormat, HeaderOrg.nWidth, HeaderOrg.nHeight, 0, texFormat, texType, compTexData);
	}

	glUniform1i(glGetUniformLocation(glProgram, "tex"), 0);
	glGenerateMipmapEXT(GL_TEXTURE_2D);

	// NOTE: need a better way, this doesn't always work well
	pDistP = (max_y-min_y)/2.0f + min_y;
	pDist  = sqrt(max_x*max_x + /*2.5f**/1.5f*max_y*max_y + max_z*max_z);
}

//-----------------------------------------------------------------------------
void GLItemViewer::draw(void) {
	if(!bShadersBuilt) BuildShaders();

	if(!valid()) {
		valid(1);
		glViewport(0, 0, pixel_w(), pixel_h());
	}

	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	static glm::mat4 model = {};
	model = glm::rotate(model, (float)(M_PI/100.0), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 proj;
	proj = glm::perspective(45.0f, (float)pixel_w()/(float)pixel_h(), 0.1f, 500.0f);

	glm::mat4 view = glm::lookAt(
		glm::vec3(pDist, pDistP, pDist),
		glm::vec3( 0.0f, pDistP,  0.0f),
		glm::vec3( 0.0f,   1.0f,  0.0f)
	);

	glUniformMatrix4fv(glUniModel, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv( glUniView, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv( glUniProj, 1, GL_FALSE, glm::value_ptr(proj));

	if(eType == ITEM_TYPE_PLUG) {
		glDrawElements(GL_TRIANGLES, m_iMaxNumIndices0, GL_UNSIGNED_INT, 0);
	} else if(eType == ITEM_TYPE_PART) {
		glDrawElements(GL_TRIANGLES, 3*m_nFC, GL_UNSIGNED_INT, 0);
	} else if(eType==ITEM_TYPE_ICONONLY || eType==ITEM_TYPE_GOLD || eType==ITEM_TYPE_SONGPYUN) {
		glm::mat4 iden = {};
		glUniformMatrix4fv(glUniModel, 1, GL_FALSE, glm::value_ptr(iden));
		glUniformMatrix4fv( glUniView, 1, GL_FALSE, glm::value_ptr(iden));
		glUniformMatrix4fv( glUniProj, 1, GL_FALSE, glm::value_ptr(iden));
		glDrawElements(GL_TRIANGLES, 3*2, GL_UNSIGNED_INT, 0);
	}
}

//-----------------------------------------------------------------------------
void GLItemViewer::Clear(void) {
	delete m_pVertices;
	delete m_pwVtxIndices;
	delete m_pfUVs;
	delete m_pwUVsIndices;
	delete compTexData;
	delete m_pIndices0;
	delete m_pVertices0;
	memset(&HeaderOrg, 0x00, sizeof(_N3TexHeader));

	eType              = ITEM_TYPE_UNKNOWN;
	m_nFC              = 0;
	m_pVertices        = nullptr;
	m_pwVtxIndices     = nullptr;
	m_pfUVs            = nullptr;
	m_pwUVsIndices     = nullptr;
	compTexData        = nullptr;
	compTexSize        = 0;
	iPixelSize         = 0;
	m_pIndices0        = nullptr;
	m_pVertices0       = nullptr;
	m_iMaxNumIndices0  = 0;
	m_iMaxNumVertices0 = 0;
}

//-----------------------------------------------------------------------------
void GLItemViewer::RenderItem(ItemInfo* item, e_Race race) {
	Clear();
	if(race == RACE_UNKNOWN) return;

	std::string mesh_file = item->getItemMeshFileForRace(race);
	e_ItemType type = item->getItemType();

	char filename[0xFFFF] = "";
	if(!mesh_file.empty()){
		strcpy(filename, mesh_file.c_str());

		size_t len_fn = strlen(filename);
		char* exten = &filename[len_fn-7];

		if(len_fn >= 7 && !strcmp(exten, "n3cplug")) {

			eType = ITEM_TYPE_PLUG;

			printf("filename: %s\n", filename);
			FILE* pFile = fopen(filename, "rb");
			if(pFile == nullptr) {
				fprintf(stderr, "ERROR: Missing N3Plug %s\n", filename);
				return;
			}

			CN3CPlug_Load(pFile);
			fclose(pFile);
		} else if(len_fn >= 7 && !strcmp(exten, "n3cpart")) {

			eType = ITEM_TYPE_PART;

			printf("filename: %s\n", filename);
			FILE* pFile = fopen(filename, "rb");
			if(pFile == nullptr) {
				fprintf(stderr, "ERROR: Missing N3Part %s\n", filename);
				return;
			}

			CN3CPart_Load(pFile);
			fclose(pFile);
		} else if(type==ITEM_TYPE_ICONONLY || type==ITEM_TYPE_GOLD || type==ITEM_TYPE_SONGPYUN) {
			// TODO: need to generate a list of these textures as well...
			eType = type;
			N3LoadTexture(mesh_file.c_str());
		} else {
			printf("Item does not have mesh.\n");
			return;
		}

		PushDataToGPU();
		//redraw();
	}
}
