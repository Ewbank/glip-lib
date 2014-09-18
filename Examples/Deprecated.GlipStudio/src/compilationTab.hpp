#ifndef __GLIPSTUDIO_COMPILATIONTAB__
#define __GLIPSTUDIO_COMPILATIONTAB__

	#include "GLIPLib.hpp"
	#include "dataModules.hpp"
	#include "codeEditor.hpp"
	#include "openSaveInterface.hpp"
	#include "titleBar.hpp"
	
	#include <QListWidget>
	#include <QVBoxLayout>

	using namespace Glip;

	class ModuleDocumentation : public Window
	{
		Q_OBJECT

		private : 
			QVBoxLayout	layout;
			QHBoxLayout 	moduleChoiceLine;
			QLabel		title;
			QComboBox	comboBox;			
			CodeEditor	description;

			QStringList	moduleNames,
					moduleInfo,
					moduleManuals;

		private slots :
			void updateDocumentationDisplay(const QString& moduleName);

		public : 
			ModuleDocumentation(QWidget* parent=NULL);
			~ModuleDocumentation(void);

			bool isDocumented(const QString& moduleName) const;
			bool isEmpty(void) const;
			void update(const LayoutLoader& loader);
	};

	class CompilationTab : public Module
	{
		Q_OBJECT

		private : 
			QVBoxLayout		layout;
			QListWidget		data;
			QAction			*showDocumentationAction,
						*dumpPipelineCodeAction;
			QMenuBar		menuBar;
			ModuleDocumentation	documentation;
			OpenSaveInterface	openSaveInterface;

			void updateDocumentation(const LayoutLoader& loader);
			void cleanCompilationTab(bool writeNoPipeline=false);
			void preparePipelineLoading(LayoutLoader& loader, const LayoutLoader::PipelineScriptElements& infos);

		private slots : 
			void pipelineWasCreated(void);
			void pipelineCompilationFailed(const Exception& e);
			void showDocumentation(void);
			void dumpPipelineCode(void);

		protected :
			void closeEvent(QCloseEvent *event);

		public : 
			CompilationTab(ControlModule& _masterModule, QWidget* parent=NULL);
			~CompilationTab(void);
	};

#endif

