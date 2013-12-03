CONFIG 		+= 	qt

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QT           	+= 	opengl
INCLUDEPATH  	+= 	/usr/local/lib \
			../ExternalTools/Qt \
			../ExternalTools/NetPBM \
			../../GLIP-Lib/include

unix: LIBS      += 	../../GLIP-Lib/lib/libglip.so
win32:Debug:	LIBS +=	../../Project_VS/GLIP-Lib/x64/Debug/GLIP-Lib.lib
win32:Release:	LIBS +=	../../Project_VS/GLIP-Lib/x64/Release/GLIP-Lib.lib

HEADERS      	+= 	./src/glSceneWidget.hpp \
			../ExternalTools/NetPBM/netpbm.hpp \
			../ExternalTools/Qt/ResourceLoader.hpp \
			./src/dataModules.hpp \
			./src/codeEditor.hpp \
			./src/codePannel.hpp \
			./src/keywords.hpp \
			./src/imageObject.hpp \
			./src/settingsManager.hpp \
			./src/openSaveInterface.hpp \
			./src/imagesCollection.hpp \
			./src/resourcesTab.hpp \
			./src/ioTab.hpp \
			./src/compilationTab.hpp \
			./src/uniformsTab.hpp \
			./src/batchTab.hpp \
			./src/libraryInterface.hpp \
			./src/mainInterface.hpp
SOURCES      	+= 	./src/main.cpp \
			./src/glSceneWidget.cpp \
			../ExternalTools/NetPBM/netpbm.cpp \
			../ExternalTools/Qt/ResourceLoader.cpp \
			./src/dataModules.cpp \
			./src/codeEditor.cpp \
			./src/codePannel.cpp \
			./src/keywords.cpp \
			./src/imageObject.cpp \
			./src/settingsManager.cpp \
			./src/openSaveInterface.cpp \
			./src/imagesCollection.cpp \
			./src/resourcesTab.cpp \
			./src/ioTab.cpp \
			./src/compilationTab.cpp \
			./src/uniformsTab.cpp \
			./src/batchTab.cpp \
			./src/libraryInterface.cpp \
			./src/mainInterface.cpp

ICON		= 	../../Doc/Doxygen/Graphics/gliplibLogo.svg

