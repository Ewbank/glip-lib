#include "imageObject.hpp"
#include "netpbm.hpp"
#include <QFileInfo>
#include <QMessageBox>

// ImageObject
	ImageObject::ImageObject(const QString& _filename)
	 : 	virtualImage(false),
		saved(false),
		filename(_filename), 
		imageBuffer(NULL),
		textureFormat(1, 1, GL_RGB, GL_UNSIGNED_BYTE),
		textureData(NULL)
	{
		try
		{
			// Open the file and guess the texture format : 
			QFileInfo file(filename);
			
			if(file.completeSuffix()=="ppm" || file.completeSuffix()=="pgm")
				imageBuffer 	= NetPBM::loadNetPBMFile(filename.toStdString());
			else if(file.completeSuffix()=="raw")
			{
				std::string comment;

				imageBuffer 	= ImageBuffer::load(filename.toStdString(), &comment);

				if(!comment.empty())
				{
					std::cout << "Comment in RAW file : " << std::endl;
					std::cout << comment << std::endl;
				}
			}
			else
			{
				QImage qimage(filename);

				if(qimage.isNull())
					throw Exception("Cannot load image \"" + filename.toStdString() + "\".", __FILE__, __LINE__);

				imageBuffer = createImageBufferFromQImage(qimage);
			}

			// Copy the format which can be written :
			textureFormat 	= (*imageBuffer);
		}
		catch(Exception& e)
		{
			delete imageBuffer;

			throw e;
		}
		catch(std::exception& e)
		{
			delete imageBuffer;

			throw Exception(e.what(), __FILE__, __LINE__);
		}
	}

	ImageObject::ImageObject(HdlTexture& texture)
	 : 	virtualImage(true),
		imageBuffer(NULL),
		textureFormat(texture),
		textureData(NULL),
		saved(false)
	{
		// Create a buffer : 
		imageBuffer = new ImageBuffer(texture);
	}

	ImageObject::~ImageObject(void)
	{
		delete imageBuffer;
		imageBuffer = NULL;
		delete textureData;
		textureData = NULL;
	}	

	bool ImageObject::isVirtual(void) const
	{
		return virtualImage;
	}

	bool ImageObject::wasSaved(void) const
	{
		return saved;
	}

	bool ImageObject::isOnDevice(void) const
	{
		return textureData!=NULL;
	}

	void ImageObject::loadToDevice(void)
	{
		textureData = new HdlTexture(textureFormat);
		(*imageBuffer) >> (*textureData);
	}

	void ImageObject::unloadFromDevice(void)
	{
		delete textureData;
		textureData = NULL;
	}

	void ImageObject::setMinFilter(GLenum mf)
	{
		textureFormat.setMinFilter(mf);

		if(textureData!=NULL)
			textureData->setMinFilter(mf);
	}

	void ImageObject::setMagFilter(GLenum mf)
	{
		textureFormat.setMagFilter(mf);

		if(textureData!=NULL)
			textureData->setMagFilter(mf);
	}

	void ImageObject::setSWrapping(GLint m)
	{
		textureFormat.setSWrapping(m);

		if(textureData!=NULL)
			textureData->setSWrapping(m);
	}

	void ImageObject::setTWrapping(GLint m)
	{
		textureFormat.setTWrapping(m);

		if(textureData!=NULL)
			textureData->setTWrapping(m);
	}

	const QString& ImageObject::getFilename(void) const
	{
		return filename;
	}

	void ImageObject::setFilename(const QString& newFilename)
	{
		filename = newFilename;
	}

	QString ImageObject::getName(void) const
	{
		if(name.isEmpty())
		{
			QFileInfo info(filename);
			return info.fileName();
		}
		else
			return name;
	}

	void ImageObject::setName(const QString& newName)
	{
		name = newName;
	}

	const __ReadOnly_HdlTextureFormat& ImageObject::getFormat(void) const
	{
		return textureFormat;
	}

	HdlTexture& ImageObject::texture(void)
	{
		if(textureData==NULL)
			throw Exception("ImageObject::texture - Texture is not currently on device.", __FILE__, __LINE__);
		else
			return *textureData;
	}

	void ImageObject::save(const std::string& targetFilename)
	{
		QImage* qimage = NULL;

		try
		{
			// Open the file and guess the texture format : 
			QFileInfo file(targetFilename.c_str());
			
			if(file.completeSuffix()=="ppm" || file.completeSuffix()=="pgm")
				NetPBM::saveNetPBMToFile(*imageBuffer, targetFilename);
			else if(file.completeSuffix()=="raw")
				imageBuffer->write(targetFilename, "Written by GlipStudio.");
			else
			{
				qimage = createQImageFromImageBuffer(*imageBuffer);
				
				const bool test = qimage->save(targetFilename.c_str());

				delete qimage;

				if(!test)
					throw Exception("Cannot save image to file \"" + targetFilename + "\".", __FILE__, __LINE__);
			}
	
			// Save filename : 
			setFilename( targetFilename.c_str() );

			saved = true;
		}
		catch(Exception& e)
		{
			delete qimage;

			throw e;
		}
		catch(std::exception& e)
		{
			delete qimage;

			throw Exception(e.what(), __FILE__, __LINE__);
		}
	}

	void ImageObject::save(void)
	{
		if(filename.isEmpty())
			throw Exception("ImageObject::save - Cannot save with an empty filename.", __FILE__, __LINE__);
		else
			this->save(filename.toStdString());
	}

	void ImageObject::replaceBy(HdlTexture& texture)
	{
		virtualImage 	= true;
		saved		= false;

		delete imageBuffer;
		imageBuffer	= new ImageBuffer(texture);

		textureFormat 	= texture;

		delete textureData;
		textureData	= NULL;
	}

