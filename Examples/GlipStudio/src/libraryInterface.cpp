#include "libraryInterface.hpp"
#include <algorithm>

	using namespace Glip::CoreGL;
	using namespace Glip::CorePipeline;

// LibraryInterface :
	LibraryInterface::LibraryInterface(ControlModule& _masterModule, QWidget *parent)
	 : 	Module(_masterModule, parent), layout(this), tabs(this), 
		resourceTab(_masterModule, this), 
		compilationTab(_masterModule, this), 
		ioTab(_masterModule, resourceTab.getResourcesManagerLink(), this),
		uniformsTab(_masterModule, this)
	{
		// Layout : 
		tabs.addTab(&resourceTab, 	"   Resources   ");	tabs.setTabTextColor(0, Qt::white);
		tabs.addTab(&compilationTab,	"   Compilation   ");	tabs.setTabTextColor(1, Qt::white);
		tabs.addTab(&ioTab,		"   I/O   ");		tabs.setTabTextColor(2, Qt::white);
		tabs.addTab(&uniformsTab, 	"   Uniforms   ");	tabs.setTabTextColor(3, Qt::white);
		layout.addWidget(&tabs);
	}

	LibraryInterface::~LibraryInterface(void)
	{ }

	void LibraryInterface::pipelineWasCreated(void)
	{
		tabs.setTabTextColor(1, Qt::white);
		tabs.setTabTextColor(2, Qt::green);
	}

	void LibraryInterface::pipelineCompilationFailed(const Exception& e)
	{
		tabs.setCurrentIndex(tabs.indexOf(&compilationTab));

		tabs.setTabTextColor(1, QColor(255, 128, 0));
		tabs.setTabTextColor(2, Qt::white);
	}

