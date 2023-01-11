HelloGlobe Example
------------------

This example would show you how to use Pyxis API.
This demo would show how to use the API in 3 steps:

1) Starting the Pyxis.Core.Engine
2) Showing your first globe using Pyxis.UI.PyxisView 
3) Allow the user to draw boxes on the globe using Pyxis.UI.Layer.SelectionLayer


Files to Look at
----------------

1) Program.cs - How to initialize Pyxis.Core.Engine services
2) Form1.cs - How to initialize a globe and Bing imagery.
              Moreover, this would show you how to create selections on the globe as well.

Creating the Engine
-------------------

The first thing to do if you want to use Pyxis API is to start Pyxis.Core.Engine.
An engine gets created using a Pyxis.Core.EngineConfig.

To make things easy for developers, we allow you to generate a default config using your API key
by using Pyxis.Core.EngineConfig.FromApiKey(ApiKey).

Once we have an engine created you would need to call engine Start() to load our SDK into memory.

When you are done with the API, calling Stop() would stop the API and deallocate the resources being used.

Creating a Globe
----------------

It starts to be fun once you see the globe. Pyxis.UI.PyxisView allows you get the power of our 
C++ 3D OpenGL renderer of any geospatial data imported using the engine.

However, PyxisView is aimed to empower the developers to customize how the globe would look like and add 
their own touch and features on top Pyxis' globe.

PyxisView contains list of Layers that render one on top the other. usually you would have 4 layers:
1) Pyxis.UI.Layers.BackgroundLayer - make sure you get the right background color you want.
2) Pyxis.UI.Layers.GlobeLayer - render a globe with elevation, coverages, polygons and icons.
3) Pyxis.UI.Layers.NavigationsControlsLayer - render navigation controls.
4) Pyxis.UI.Layers.SelectionLayer - allow users to make selections on the globe.

Use Pyxis.UI.PyxisViewExtensions.CreateGlobe helper function to create those 4 layers in one line.

You don't have to create those layers and you can add custom layers to create the user experience 
you need. Please note you can use any WinForms control on top the PyxisView. 

Here is some information to decide between the 2 options:
1) Use WinForms controls on top of PyxisView - use this option when you want to create simple pop-up
   and controls that do not require complex rendering. This is the easiest option to add controls
   on top of the globe.

2) Use CustomLayers - use this option when you want to use OpenGL capabilities on top of the globe.
   This is useful when you want to draw semitransparent elements on the globe or take advantage
   of 3d rendering. Adding a layer would also integrate with SaveToBitmap() function of PyxisView
   which can be used for printing. Custom layers can also allow you to change the logic of
   Mouse and Keyboard events but this topic is out of the scope of this demo.

This demo would show you how to use WinForms button on top PyxisView.
