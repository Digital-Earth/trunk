using Pyxis.Contract.Publishing;
using Pyxis.Core;
using Pyxis.Core.IO;
using Pyxis.Core.IO.GeoJson;
using Pyxis.Core.IO.GeoJson.Specialized;
using Pyxis.Core.Measurements;
using Pyxis.UI.Layers;
using Pyxis.Utilities;
using Pyxis.WorldView.Studio.Layers;
using Pyxis.WorldView.Studio.Layers.Html;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace Pyxis.WorldView.Studio.JsAPI
{
    internal class GlobeAPI
    {
        public ApplicationForm Form { get; private set; }

        public GlobeLayer GlobeLayer
        {
            get
            {
                return Form.GlobeLayer;
            }
        }

        public Engine Engine
        {
            get
            {
                return Form.Engine;
            }
        }

        public GlobeAPI(ApplicationForm form)
        {
            Form = form;
        }

        public void Register(HtmlLayer HtmlLayer)
        {
            HtmlLayer.RegisterProxy("globe", proxy =>
            {
                proxy.Bind("renderReport", () => { return Form.PyxisView.RenderingReport; });
                proxy.Bind("cursor", () => { return GlobeLayer.GetCursorIndex().toString(); });
                proxy.Bind("pickArea", (int pixelWidth) =>
                {
                    var pointerIndex = GlobeLayer.GetCursorIndex();
                    if (pointerIndex.isNull())
                    {
                        return null;
                    }
                    else
                    {
                        var cellCircle = PYXCell.create(pointerIndex).getBoundingCircle();

                        return new CircleGeometry()
                        {
                            Coordinates = new GeographicPosition(PointLocation.fromPYXIndex(pointerIndex)),
                            Radius = cellCircle.getRadius() * pixelWidth * SphericalDistance.Radian
                        };
                    }
                });
                proxy.BindAsync("show", (GeoSource geoSource) =>
                {
                    if (Engine.GetProcess(geoSource) == null)
                    {
                        throw new Exception("Failed to initialize GeoSource");
                    }
                    return GlobeLayer.ViewState.Show(geoSource).ToString();
                });
                proxy.BindAsync("showWithStyle", (GeoSource geoSource, Style style) =>
                {
                    if (Engine.GetProcess(geoSource) == null)
                    {
                        throw new Exception("Failed to initialize GeoSource");
                    }
                    return GlobeLayer.ViewState.Show(geoSource, style).ToString();
                });

                proxy.BindAsync("createDefaultStyle", (GeoSource geoSource, Style style) =>
                {
                    if (Engine.GetProcess(geoSource) == null)
                    {
                        throw new Exception("Failed to initialize GeoSource");
                    }
                    return GlobeLayer.ViewState.CreateDefaultStyle(geoSource, style);
                });

                proxy.BindAsync("getVisibleId", (GeoSource geoSource) =>
                {
                    var id = GlobeLayer.ViewState.GetStyledGeoSource(geoSource);
                    //JSValue as null throws an exception, therefore we return empty string
                    return (!id.HasValue) ? "" : id.Value.ToString();
                });
                proxy.BindAsync("hide", (string id) =>
                {
                    GlobeLayer.ViewState.Hide(Guid.Parse(id));
                    return id;
                });
                proxy.BindAsync("getStyle", (string id) =>
                {
                    return GlobeLayer.ViewState.GetStyle(Guid.Parse(id));
                });
                proxy.BindAsync("setStyle", (string id, Style style) =>
                {
                    GlobeLayer.ViewState.ApplyStyle(Guid.Parse(id), style);
                    return id;
                });
                proxy.Bind("isVisibleIdLoading", (string id) =>
                {
                    return GlobeLayer.IsLoadingByVisibleId(Guid.Parse(id));
                });

                proxy.BindAsync("getAllLoadingVisibleIds", () =>
                {
                    var result = new List<string>();
                    if (GlobeLayer.IsLoading)
                    {
                        result.AddRange(from id in GlobeLayer.ViewState.GetStyledGeoSourcesIds()
                                        where GlobeLayer.IsLoadingByVisibleId(id)
                                        select id.ToString());
                    }
                    return result;
                });

                proxy.BindAsync("setStyleByField", (string id, string fieldName) =>
                {
                    return GlobeLayer.ViewState.SetStyleByField(Guid.Parse(id), fieldName);
                });

                proxy.BindAsync("setStyleByFieldWithPalette", (string id, string fieldName, Palette palette) =>
                {
                    return GlobeLayer.ViewState.SetStyleByField(Guid.Parse(id), fieldName, palette);
                });

                proxy.BindAsync("setStyleByFieldWithPaletteForScreen", (string id, string fieldName, Palette palette) =>
                {
                    return GlobeLayer.ViewState.SetStyleByFieldBasedOnGeometry(Guid.Parse(id), fieldName, palette, GlobeLayer.GetScreenGeometry());
                });

                proxy.BindAsync("getLayers", () =>
                {
                    return GlobeLayer.ViewState.GetStyledGeoSourcesIds();
                });

                proxy.BindAsync("getGeoSources", () =>
                {
                    return GlobeLayer.ViewState.GetGeoSources();
                });

                proxy.BindAsync("gotoGeometry", (IGeometry geometry, int duration) =>
                {
                    var currentCamera = GlobeLayer.GetCamera();
                    GlobeLayer.GotoGeometry(geometry, TimeSpan.FromMilliseconds(duration));
                    Task.Delay(duration).Wait();
                    return currentCamera;
                });

                proxy.BindAsync("gotoGeoSource", (GeoSource geosource, int duration) =>
                {
                    var currentCamera = GlobeLayer.GetCamera();
                    GlobeLayer.GotoPipeline(geosource, TimeSpan.FromMilliseconds(duration));
                    Task.Delay(duration).Wait();
                    return currentCamera;
                });

                proxy.Bind("getCamera", () =>
                {
                    return GlobeLayer.GetCamera();
                });

                proxy.BindAsync("getCameraAsync", () =>
                {
                    return GlobeLayer.GetCamera();
                });

                proxy.BindAsync("setCamera", (Pyxis.Contract.Publishing.Camera camera, int duration) =>
                {
                    var result = Form.InvokeIfRequired(() =>
                    {
                        var currentCamera = GlobeLayer.GetCamera();
                        GlobeLayer.SetCamera(camera, TimeSpan.FromMilliseconds(duration));
                        return currentCamera;
                    });
                    Task.Delay(duration).Wait();
                    return result;
                });

                proxy.BindAsync("screenToGeographicPosition", (List<int[]> screenLocations) =>
                {
                    return Form.InvokeIfRequired(() => GlobeLayer.ScreenToGeographicPosition(screenLocations));
                });

                //Mouse events on the globe are simple mouse click because we want the left button to be used for click and drag at the same time.
                //therefore, there is a logic that block mouse click events to JS if we detected mouse drag while clicking.

                var click = proxy.Callback<string, JsMouseEvent>("click");
                var rightClick = proxy.Callback<string, JsMouseEvent>("rightClick");
                var dragstart = proxy.Callback<string, JsMouseEvent>("dragstart");

                var dragging = false;
                Point mouseDownLocation = new Point();
                const int MaximumMovementInPixelsToBeConderiderDragging = 5;
                
                //this event track where the user did mouse down event. 
                GlobeLayer.GlobeMouseDown += (s, e) =>
                {
                    dragging = false;
                    mouseDownLocation = e.Location;
                };

                //if the user moved the mouse while a button is pressed more then 5 pixel in any direction, we mark this mouse click as dragging
                GlobeLayer.GlobeMouseMove += (s, e) =>
                {
                    if (e.Button != MouseButtons.None && !dragging)
                    {
                        var moveDistance = Math.Sqrt(Math.Pow(mouseDownLocation.X - e.X, 2) + Math.Pow(mouseDownLocation.Y - e.Y, 2));
                        if (moveDistance > MaximumMovementInPixelsToBeConderiderDragging)
                        {
                            dragging = true;
                            var mouseEvent = new JsMouseEvent { clientX = e.X, clientY = e.Y };
                            
                            //notify JS that drag started
                            dragstart(GlobeLayer.GetCursorIndex().toString(), mouseEvent);
                        }
                    }
                };


                GlobeLayer.GlobeMouseClick += (s, e) =>
                {
                    //we only trigger mouse events if we are not dragging
                    if (!dragging)
                    {
                        var mouseEvent = new JsMouseEvent { clientX = e.X, clientY = e.Y };
                        if (e.Button == MouseButtons.Left)
                        {
                            click(GlobeLayer.GetCursorIndex().toString(), mouseEvent);
                        }
                        else if (e.Button == MouseButtons.Right)
                        {
                            rightClick(GlobeLayer.GetCursorIndex().toString(), mouseEvent);
                        }
                    }
                };

                proxy.BindAsync("capture", (ScreenCaptureArgs args) =>
                {
                    var bitmap = Form.PyxisView.SaveToBitmap();
                    var croppedImage = bitmap.Clone(new Rectangle(new Point(args.Left, args.Top), new Size(args.Width, args.Height)), bitmap.PixelFormat);
                    var url = Form.ApplicationJsAPI.ImageStorage.ToResourceName(Form.ApplicationJsAPI.ImageStorage.SaveImage(croppedImage));
                    return url;
                });
            });
        }
    }
}
