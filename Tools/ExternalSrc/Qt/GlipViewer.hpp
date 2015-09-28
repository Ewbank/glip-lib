#ifndef __GLIP_VIEWER__
#define __GLIP_VIEWER__

	#include "GLIPLib.hpp"
	#include "GlipResource.hpp"
	#include <cmath>
	#include <QGLWidget>
	#include <QGraphicsScene>
	#include <QGraphicsView>

	#include <QGraphicsItemGroup>
	//#include <QGraphicsWidget>
	#include <QGraphicsProxyWidget>
	//#include <QGraphicsGridLayout>

	#include <QGridLayout>
	//#include <QHBoxLayout>
	#include <QLabel>
	#include <QGraphicsSceneMouseEvent>
	#include <QPushButton>

namespace QGlip
{
	// Propotypes :
	class View;
	
	// Objects :
	class TopBar : public QGraphicsProxyWidget
	{
		Q_OBJECT	

		public :
			enum Position
			{
				Left		= 0,
				Center		= 1,
				Right		= 2,
				NumPositions 	= 3
			};

		private :
			QWidget			widget;
			QGridLayout		layout;
	
			QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);
		
		private slots :
			void fitToScene(const QRectF& newRect);

		public :
			TopBar(QGraphicsItem* parent=NULL);
			virtual ~TopBar(void);
			QWidget* getWidget(const Position& p);
			QWidget* takeWidget(const Position& p);
			void setWidget(const Position& p, QWidget* ptr); // Not taking ownership.
			static int getHeight(void);
	};

	class ViewTransform : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(QPointF center MEMBER center READ getCenter WRITE setCenter)
		Q_PROPERTY(float angle_rad MEMBER angle_rad READ getAngle_rad WRITE setAngle_rad)
		Q_PROPERTY(float scale MEMBER scale READ getScale WRITE setScale)
		Q_PROPERTY(bool verticalFlip MEMBER verticalFlip READ getVerticalFlip WRITE setVerticalFlip)
		Q_PROPERTY(bool horizontalFlip MEMBER horizontalFlip READ getHorizontalFlip WRITE setHorizontalFlip)
	
		private :
			QPointF	center;
			float	angle_rad,
				scale;
			bool	verticalFlip,
				horizontalFlip;
			float	c, s;	

		public :
			ViewTransform(void);
			ViewTransform(const ViewTransform& t);
			virtual ~ViewTransform(void);
			ViewTransform& operator=(const ViewTransform& t);
			QPointF getCenter(void) const;
			void setCenter(const QPointF& c);
			float getAngle_rad(void) const;
			float getAngleCosine(void) const;
			float getAngleSine(void) const;
			void setAngle_rad(const float& a);
			float getScale(void) const;
			void setScale(const float& s);
			bool getVerticalFlip(void) const;
			void setVerticalFlip(const bool& e);
			bool getHorizontalFlip(void) const;
			void setHorizontalFlip(const bool& e); 
			QPointF standardQuadVertexToGLCoordinates(const QPointF& vertex, const QSize& sceneSize, const QSize& textureSize) const;
			QPointF pixelToFragmentCoordinates(const QPointF& pixel, const QRectF& sceneRect, const QSize& textureSize, const bool& omitFlips=false) const;
			void reset(const QSize& textureSize, const int& duration=0);
			void translate(const QPointF& t);
			void rotate(const float& a);
			void zoom(const float& r, const QPointF& p);
			void zoom(const float& r);
			void scaleOneToOne(void);
			static float getBaseScale(const QSize& sceneSize, const QSize& textureSize);
			float getCompoundScale(const QSize& sceneSize, const QSize& textureSize) const;
			void flipHorizontal(void);
			void flipVertical(void);

		signals :
			void transformModified(void);
	};	

	class AbstractGLDrawableObject : public QObject
	{
		Q_OBJECT	

		protected :
			AbstractGLDrawableObject(QObject* parent=NULL);
			void processMousePressEvent(const QRectF& sceneRect, QGraphicsSceneMouseEvent* event, const View* view);
			void processMouseMoveEvent(const QRectF& sceneRect, QGraphicsSceneMouseEvent* event, View* view, View** subViews=NULL, const int numSubViews=0);
			void processWheelEvent(const QRectF& sceneRect, QGraphicsSceneWheelEvent* event, View* view, View** subViews=NULL, const int numSubViews=0);
			void processKeyPressEvent(const QRectF& sceneRect, QKeyEvent* event, View* view, View** subViews=NULL, const int numSubViews=0);

		public :
			virtual QGraphicsItem* getUnderlyingItem(void) = 0;
			virtual void populateTopBar(TopBar& bar) = 0;
			virtual void drawBackground(const QRectF& rect, Glip::CoreGL::HdlProgram& program, Glip::CorePipeline::GeometryInstance& quad) = 0;
			void showObject(void);	
			virtual QList<AbstractGLDrawableObject*> getSubObjects(void) const;

		signals :
			void showRequest(void);
			void closed(void);
			void updateScene(void);
			void gotoPreviousObject(void);
			void gotoNextObject(void);
			void gotoParentObject(void);
			void addSubObject(AbstractGLDrawableObject*);
	};
	
	class View : public AbstractGLDrawableObject, public QGraphicsItem
	{
		Q_OBJECT

		private :
			TextureResource	*textureResource;
			QLabel		title;
			QWidget		controlsWidget;
			QHBoxLayout	controlsLayout;
			QPushButton	previousButton,
					parentButton,
					nextButton,
					closeButton;	

		private slots :
			void updateTitle(void);
			void resourceDestroyed(void);		

		private :	
			void mousePressEvent(QGraphicsSceneMouseEvent* event);
			void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
			void wheelEvent(QGraphicsSceneWheelEvent* event);
			void keyPressEvent(QKeyEvent* event);

		public :
			ViewTransform transform;

			View(TextureResource* ptr=NULL, QObject* parent=NULL);
			virtual ~View(void);
			const QSize getTextureSize(void) const;
			TextureResource* getTextureResource(void) const;
			QPointF standardQuadVertexToGLCoordinates(const QPointF& vertex, const QSize& rect) const;
			QPointF standardQuadVertexToGLCoordinates(const QPointF& vertex) const;
			QPointF pixelToFragmentCoordinates(const QPointF& pixel, const QRectF& rect, const bool& omitFlips=false) const;
			QPointF pixelToFragmentCoordinates(const QPointF& pixel, const bool& omitFlips=false) const;
			void resetTransform(const int& duration=0);
			float getBaseScale(const QSize& sceneSize) const;
			float getCompoundScale(const QSize& sceneSize) const;
			// From AbstractGLDrawableObject :
			QGraphicsItem* getUnderlyingItem(void);
			void populateTopBar(TopBar& bar);
			void drawBackground(const QRectF& rect, Glip::CoreGL::HdlProgram& program, Glip::CorePipeline::GeometryInstance& quad, ViewTransform* t);
			void drawBackground(const QRectF& rect, Glip::CoreGL::HdlProgram& program, Glip::CorePipeline::GeometryInstance& quad);
			// From QGraphicsItem :
			QRectF boundingRect(void) const;
			void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	};
	
	class Vignette : public QObject, public QGraphicsItemGroup
	{
		Q_OBJECT
		Q_PROPERTY(float frameWidth MEMBER frameWidth)
		Q_PROPERTY(float frameHighlightedWidth MEMBER frameHighlightedWidth)
		Q_PROPERTY(QColor frameColor MEMBER frameColor)
		Q_PROPERTY(QColor frameHighlightedColor MEMBER frameHighlightedColor)
		Q_PROPERTY(QColor titleBackgroundColor MEMBER titleBackgroundColor)

		private :
			View*			view;
			TextureResource*	textureResource;
			QGraphicsRectItem	frame,
						titleBar;
			QGraphicsSimpleTextItem	title;
			float			frameWidth,
						frameHighlightedWidth;
			QColor			frameColor,
						frameHighlightedColor,
						titleBackgroundColor;

		private slots :
			void updateTitle(void);
			void viewDestroyed(void);
			void resourceDestroyed(void);
	
		public :
			Vignette(View* ptr, QObject* parent=NULL);
			virtual ~Vignette(void);
			View* getView(void) const;
			QRectF boundingRect(void) const;
			void resize(const QSize& s);
			void enableSelectionHighlight(const bool& enable);
	};

	class Gallery : public AbstractGLDrawableObject, public QGraphicsItemGroup
	{
		Q_OBJECT

		private :
			QList<QPair<View*, Vignette*> >	views;	// Preserve input order
			int				numColumns,
							numRows;
			QSize				vignetteSize;
			float				horizontalSpacing,
							verticalSpacing;
			QVector<Vignette*>		selection;
			QVector<View*>			selectionViews;

			QList<QPair<View*, Vignette*> >::iterator getIterator(const View* view);
			bool contains(const TextureResource* resource) const;
			void computeTableParameters(const QSize& size, int N=0);
			QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value);
			void updateSelection(const QPointF& pos, const bool& add, const bool& remove, const bool& clear);
			void mousePressEvent(QGraphicsSceneMouseEvent* event);
			//void mouseReleaseEvent(QGraphicsSceneMouseEvent* event);
			void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);
			void mouseMoveEvent(QGraphicsSceneMouseEvent* event);
			void wheelEvent(QGraphicsSceneWheelEvent* event);
			void keyPressEvent(QKeyEvent* event);
	
		private slots :
			void processGotoNextView(void);
			void processGotoPreviousView(void);
			void processGotoParentGallery(void);
			void viewClosed(void);
			void resize(const QRectF& rect);
			void resize(void);

		public :
			Gallery(QObject* parent=NULL);
			virtual ~Gallery(void);
			void addView(TextureResource* resource, const bool& allowDuplicate=false);
			// From AbstractGLDrawableObject :
			QGraphicsItem* getUnderlyingItem(void);
			void populateTopBar(TopBar& bar);
			void drawBackground(const QRectF& rect, Glip::CoreGL::HdlProgram& program, Glip::CorePipeline::GeometryInstance& quad);
			QList<AbstractGLDrawableObject*> getSubObjects(void) const;
			// From QGraphicsItem :
			QRectF boundingRect(void) const;
	};

	class GLContext : public QGLWidget
	{
		Q_OBJECT

		private :
			bool glipOwnership;
	
			GLContext(const GLContext&);
			GLContext& operator=(const GLContext&);

		public :
			GLContext(QGLContext* ctx, QWidget* parent=NULL);
			virtual ~GLContext(void);
	};

	class GLScene : public QGraphicsScene
	{
		Q_OBJECT	
		Q_PROPERTY(QColor backgroundColor MEMBER backgroundColor READ getBackgroundColor WRITE setBackgroundColor)

		private :
			static const std::string		displayVertexShaderSource,
								displayFragmentShaderSource;
			Glip::CorePipeline::GeometryInstance	*quad;
			Glip::CoreGL::HdlProgram		*program;
			QColor					backgroundColor;
			QVector<AbstractGLDrawableObject*>	objects;
			TopBar					topBar;
	
			GLScene(const GLScene&);
			GLScene& operator=(const GLScene&);
			void drawBackground(QPainter* painter, const QRectF& rect);

		private slots :
			void showObject(void);
			void objectDestroyed(void);

		public :
			GLScene(QObject* parent=NULL);
			virtual ~GLScene(void);
			const QColor& getBackgroundColor(void) const;
			void setBackgroundColor(const QColor& c);
	
		public slots :	
			void addObject(AbstractGLDrawableObject* ptr);
	};

	class Viewer : public QGraphicsView
	{
		Q_OBJECT
	
		private :
			GLContext		*glContext;
			GLScene			*glScene;
			QVector<Gallery*>	galleries;

			void resizeEvent(QResizeEvent* event);

		private slots :
			void galleryDestroyed(void);

		public :
			Viewer(QWidget* parent=NULL);
			virtual ~Viewer(void);
			Gallery* createGallery(void);

		public slots :
			void addView(TextureResource* ptr, const bool allowDuplicate=false);
	};
}

#endif
