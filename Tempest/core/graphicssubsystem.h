#ifndef GRAPHICSSUBSYSTEM_H
#define GRAPHICSSUBSYSTEM_H

namespace Tempest {

class GraphicsSubsystem {
  public:
    GraphicsSubsystem();
    virtual ~GraphicsSubsystem();
    
    class Texture;
    class StdDSSurface;

    class IndexBuffer;
    class VertexBuffer;
    class VertexDecl;

    enum ShaderType{
      Vertex,
      Fragment/*,
      Geometry*/
      };

    class FragmentShader;
    class VertexShader;

    class Device;

    struct Event{
      Event();
      
      enum Type{
        NoEvent,
        DeleteObject,
        Count
        };

      Type type;
      };
    
    struct DeleteEvent:Event {
      DeleteEvent() {
        type = DeleteObject;

        texture     = 0;
        index       = 0;
        vertex      = 0;
        declaration = 0;
        
        fs = 0;
        vs = 0;
        }
      
      Texture*      texture;
  
      IndexBuffer*  index;
      VertexBuffer* vertex;
      VertexDecl*   declaration;
  
      FragmentShader* fs;
      VertexShader*   vs;
      };
    
    virtual void event( const Event& e );
    virtual void event( const DeleteEvent& e );
  };

}

#endif // GRAPHICSSUBSYSTEM_H
