CONFIG 	+= 	qt \
		debug

QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT += opengl

INCLUDEPATH += 	/usr/local/lib \
		../ExternalSrc/Qt \
		../../GLIP-Lib/include

unix: 		LIBS += ../../GLIP-Lib/lib/libglip.so
win32:Debug:	LIBS +=	../../Project_VS/GLIP-Lib/x64/Debug/GLIP-Lib.lib
win32:Release:	LIBS +=	../../Project_VS/GLIP-Lib/x64/Release/GLIP-Lib.lib

INCLUDEPATH +=	../ExternalSrc/Qt \
		../ExternalSrc/NetPBM

HEADERS += 	../ExternalSrc/Qt/GLSceneWidget.hpp \	
		../ExternalSrc/NetPBM/NetPBM.hpp \ 	
		../ExternalSrc/Qt/ImageItem.hpp \ 
		../ExternalSrc/Qt/PipelineManager.hpp \
		../ExternalSrc/Qt/UniformsVarsLoaderInterface.hpp \
		../ExternalSrc/Qt/GLSLKeywords.hpp \
		../ExternalSrc/Qt/CodeEditor.hpp \
		./src/GlipStudio.hpp
SOURCES += 	../ExternalSrc/Qt/GLSceneWidget.cpp \	
		../ExternalSrc/NetPBM/NetPBM.cpp \	
		../ExternalSrc/Qt/ImageItem.cpp \
		../ExternalSrc/Qt/PipelineManager.cpp \
		../ExternalSrc/Qt/UniformsVarsLoaderInterface.cpp \
		../ExternalSrc/Qt/GLSLKeywords.cpp \
		../ExternalSrc/Qt/CodeEditor.cpp \
		./src/GlipStudio.cpp \
		./src/main.cpp

