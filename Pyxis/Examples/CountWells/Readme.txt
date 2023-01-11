CountWells Example
------------------

This example would show you how to use Pyxis API.
This demo would show how to use the API in 5 steps:

1) Starting the Pyxis.Core.Engine using Pyxis.UI.PyxisEngineFactory
2) Showing your first globe using Pyxis.UI.PyxisView 
3) Importing data to the globe using reflection
4) Allow the user to select features on the globe using Pyxis.UI.Layer.SelectionLayer
5) Allow the retrieval of all features from a GeoSource inside a geometry


Files to Look at
----------------
2) Form1.cs - How to initialize a globe and well data.
              Moreover, this would show you how to create selections and filter features.

Creating the Engine
-------------------

The first thing to do if you want to use Pyxis API is to start Pyxis.Core.Engine.
An engine gets created using a Pyxis.Core.EngineConfig.

This example shows an easy way to do that when using simple WinForm application.

Drag and drop Pyxis.UI.PyxisEngineFactory component to the main form and enter
the email and API key in the settings.

Calling Pyxis.UI.PyxisEngineFactory.WhenReady(Action) or GetEngine() would initialize
the Pyxis.Core.Engine with the right settings for you.

Creating a Globe
----------------

It starts to be fun once you see the globe. Pyxis.UI.PyxisView allow you get the power of our 
C++ 3D OpenGL renderer of any geospatial data imported using the engine.

However, PyxisView is aimed to empower developers to customize how the globe would look like and add 
their own touch and features on top of the Pyxis globe.

PyxisView contains a list of Layers that render one on top the other. usually you would have 4 layers:
1) Pyxis.UI.Layers.BackgroundLayer - make sure you gut the right background color you want.
2) Pyxis.UI.Layers.GlobeLayer - render a globe with elevation, coverages, polygons and icons.
3) Pyxis.UI.Layers.NavigationsControlsLayer - render navigation controls
4) Pyxis.UI.Layers.SelectionLayer - allow users to select an area on the globe

Use Pyxis.UI.PyxisViewExtensions.CreateGlobe helper function to create those 4 layers in one line.

You don't have to create those layers and you can add custom layers to create the user experience 
you need. Please note you can use any WinForms control on top of the PyxisView. 

Here is some information to decide between the 2 options:
1) Use WinForms controls on top PyxisView - use this option when you want to create simple pop-up
   and controls that do not require complex rendering. This is the easiest option to add controls
   on top of the globe.

2) Use CustomLayers - use this option when you want to use OpenGL capabilities on top of the globe.
   This is useful when you want to draw semitransparent elements on the globe or take advantage
   of 3d rendering. Adding a layer would also integrate with SaveToBitmap() function of PyxisView
   which can be used for printing. Custom layers can also allow you to change the logic of
   Mouse and Keyboard events but this topic is out of the scope of this demo.

This demo would show you how to use WinForms button on top PyxisView.

Using Reflection
----------------

Pyxis.Core.Engine.CreateInMemory<T>(IEnumerable<T> items) is the easiest way to get data on the globe.
This function allows you to convert simple classes into features using reflection.

The items class only need to have:
1) Id property - the name of the property needs to be Id or decorated with 
   Pyxis.Core.IO.Reflection.FeatureIdAttribute.
2) Geometry property - a property that implements the Pyxis.Core.IO.IGeometry interface.

Any other public property would be treated as an attribute that can be used for styling and filtering.

Pyxis.Core.Engine.CreateInMemory<T>(IEnumerable<T> items) iterates over the items, converts them into
Pyxis.Core.IO.GeoJson.Feature objects and then creates a GeoSource in memory. The generated GeoSource 
is a snapshot of the items given to this function. The items can be changed but it won't affect
the generated GeoSource.

Filtering Data
--------------

Pyxis.Core.Engine.GetAsFeatures(GeoSource).GetFeatures(IGeometry) is a tool that allows you
to find which features intersect the given geometry.

This example would show you how to use this function to generate a summary of all features 
inside the selected area.