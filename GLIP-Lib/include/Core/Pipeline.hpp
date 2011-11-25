#ifndef __GLIPLIB_PIPELINE__
#define __GLIPLIB_PIPELINE__

    // Includes
        #include "Component.hpp"
        #include "Filter.hpp"

namespace Glip
{
    namespace CoreGL
    {
        // Prototypes
            class HdlTexture;
            class __HdlTextureFormat_OnlyData;
            class HdlFBO;
            class ShaderSource;
            class HdlShader;
            class HdlProgram;
    }

    namespace CorePipeline
    {
        // Prototypes
            class PipelineLayout;
            class Pipeline;

        // Constants
            #define ELEMENT_NOT_ASSOCIATED (-2)

            enum ComponentKind
            {
                THIS_PIPELINE = -1,
                FILTER        =  0,
                PIPELINE      =  1
            };

            // Enums
            enum ActionType
            {
                Process,
                Reset
            };

        // Types
            typedef std::vector<__ReadOnly_ComponentLayout*> ComponentList;

        // Objects
            class __ReadOnly_PipelineLayout : virtual public __ReadOnly_ComponentLayout
            {
                public :
                    // Constants
                        struct Connection
                        {
                            int idIn;
                            int portIn;
                            int idOut;
                            int portOut;
                        };
                private :
                    // Data
                        std::vector<Connection>    connections;
                        ComponentList              elementsLayout;
                        std::vector<ComponentKind> elementsKind;
                        std::vector<int>           elementsID;
                    // Friends
                        friend class PipelineLayout;
                protected :
                    // Tools
                        __ReadOnly_PipelineLayout(const std::string& type);
                        int                    	getElementID(int i);
                        void                   	setElementID(int i, int ID);
                        Connection              getConnection(int i);
                        std::vector<Connection> getConnectionDestinations(int id, int p);
                        Connection              getConnectionSource(int id, int p);
                    // Friends
                        friend class Pipeline;
                public :
                    // Tools
                        __ReadOnly_PipelineLayout(const __ReadOnly_PipelineLayout& c);
                        //~__ReadOnly_PipelineLayout(void);

                        void 				checkElement(int i) const;
                        int  				getNumElements(void);
                        int  				getNumConnections(void);
                        void				getInfoElements(int& numFilters, int& numPipelines);
                        int  				getElementIndex(const std::string& name) const;
                        ComponentKind 			getElementKind(int i) const;
                        static ObjectName&      	componentName(int i, const void* obj);
                        __ReadOnly_ComponentLayout& 	componentLayout(int i) const;
                        __ReadOnly_ComponentLayout& 	componentLayout(const std::string& name) const;

                        __ReadOnly_FilterLayout&   	filterLayout(int i) const;
                        __ReadOnly_FilterLayout&   	filterLayout(const std::string& name) const;
                        __ReadOnly_PipelineLayout& 	pipelineLayout(int i) const;
                        __ReadOnly_PipelineLayout& 	pipelineLayout(const std::string& name) const;

                        std::string 			getConnectionDestinationsName(int filterSource, int port);
                        std::string 			getConnectionDestinationsName(const std::string& filterSource, const std::string& port);
                        std::string 			getConnectionSourceName(int filterDestination, int port);
                        std::string 			getConnectionSourceName(const std::string& filterDestination, const std::string& port);
                        bool 				check(bool exception = true);
            };

            class PipelineLayout : virtual public ComponentLayout, virtual public __ReadOnly_PipelineLayout
            {
                public :
                    // Tools
                        PipelineLayout(const std::string& type);
                        PipelineLayout(__ReadOnly_PipelineLayout& c);
                        int  				add(const __ReadOnly_FilterLayout& filterLayout,     const std::string& name);
                        int  				add(const __ReadOnly_PipelineLayout& pipelineLayout, const std::string& name);
                        int  				addInput(const std::string& name);
                        int  				addOutput(const std::string& name);

                        void 				connect(int filterOut, int portOut, int filterIn, int portIn);
                        void 				connect(const std::string& filterOut, const std::string& portOut, const std::string& filterIn, const std::string& portIn);
                        void 				connectToInput(int port, int filterIn, int portIn);
                        void				connectToInput(const std::string& port, const std::string& filterIn, const std::string& portIn);
                        void 				connectToOutput(int filterOut, int portOut, int port);
                        void 				connectToOutput(const std::string& filterOut, const std::string& portOut, const std::string& port);
            };

            class Pipeline : public __ReadOnly_PipelineLayout, public Component
            {
                private :
                    // Data
                        typedef std::vector<HdlTexture*> 	TablePtr;
                        typedef std::vector<Filter*>     	TableFilter;
                        typedef std::vector<Connection>   	TableConnection;
                        TablePtr       				input;
                        TablePtr       				output;
                        TableFilter    				filters;
                        std::vector<int> 			actionFilter;
                        std::vector<TableConnection*> 		connexions;
                        std::vector<HdlFBO*>			buffers;
                    // Tools
                        void cleanInput(void);
                        void build(void);
                protected :
                    // Tools
                        void process(void);
                public :
                    // Tools
                        Pipeline(__ReadOnly_PipelineLayout& p, const std::string& name);
                        //Pipeline(const std::string& filename); TODO
                        //~Pipeline(void);

                        Pipeline&  operator<<(HdlTexture&);
                        Pipeline&  operator<<(ActionType);
                        HdlTexture& out(int);
                        HdlTexture& out(const std::string&);
                        Filter& operator[](const std::string& name);
            };
    }
}


#endif
