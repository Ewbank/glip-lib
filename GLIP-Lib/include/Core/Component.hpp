#ifndef __GLIP_COMPONENT_HPP__
#define __GLIP_COMPONENT_HPP__

    // Includes
        #include "NamingLayout.hpp"


namespace Glip
{
    namespace CoreGL
    {
        class HdlTexture;
    }

    using namespace CoreGL;

    namespace CorePipeline
    {
        // Prototypes
            class ComponentLayout;

        // Objects
            class __ReadOnly_ComponentLayout : public ObjectName
            {
                private :
                    // Data
                        std::vector<ObjectName> inputPortDescription;
                        std::vector<ObjectName> outputPortDescription;

                        friend class ComponentLayout;
                protected :
                    // Tools
                        __ReadOnly_ComponentLayout(const std::string& type);
                public :
                    // Tools
                        __ReadOnly_ComponentLayout(const __ReadOnly_ComponentLayout&);

                        void checkInputPort(int i);
                        void checkOutputPort(int i);
                        int                getNumInputPort(void);
                        const std::string& getInputPortName(int i);
                        std::string        getInputPortNameExtended(int i);
                        int                getInputPortID(const std::string& name);
                        int                getNumOutputPort(void);
                        const std::string& getOutputPortName(int i);
                        std::string        getOutputPortNameExtended(int i);
                        int                getOutputPortID(const std::string& name);
            };

            class ComponentLayout : virtual public __ReadOnly_ComponentLayout
            {
                protected :
                    // Tools
                        int addOutputPort(const std::string& name); // =""?
                        int addInputPort(const std::string& name);  // =""?
                public :
                    // Tools
                        ComponentLayout(const std::string& type);
                        ComponentLayout(const __ReadOnly_ComponentLayout&);

                        void setInputPortName(int id, const std::string& name);
                        void setOutputPortName(int id, const std::string& name);
            };


            class Component : virtual public __ReadOnly_ComponentLayout
            {
                protected :
                    // Tools
                        Component(const __ReadOnly_ComponentLayout&, const std::string& name);
            };
    }
}

#endif
