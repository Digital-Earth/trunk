using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using Tao.OpenGl;

namespace Pyxis.UI.Layers.Sprites
{
    /// <summary>
    /// The root graphics sprite in the parent-child tree of Pyxis.UI.Layers.Sprites.Sprite to display on a Pyxis.UI.ILayer.
    /// Manages the allocating and releasing of graphics library textures as well as setting up the graphics projection when drawing.
    /// </summary>
    public class RootSprite : Sprite, IGlContext
    {        
        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Sprites.RootSprite class.
        /// </summary>
        public RootSprite()
            : base(null)
        {
            Context = this;
        }

        /// <summary>
        /// Draw the Pyxis.UI.Layers.Sprites.RootSprite and all of its children.
        /// </summary>
        public override void Draw()
        {
            ReleaseTexturesIfNeeded();

            SetupProjection();

            Gl.glEnable(Gl.GL_TEXTURE_2D);
            Gl.glEnable(Gl.GL_BLEND);
            Gl.glTexEnvi(Gl.GL_TEXTURE_ENV, Gl.GL_TEXTURE_ENV_MODE, Gl.GL_MODULATE);
            Gl.glBlendFunc(Gl.GL_SRC_ALPHA, Gl.GL_ONE_MINUS_SRC_ALPHA);
            Gl.glDisable(Gl.GL_DEPTH_TEST);
            Gl.glPolygonMode(Gl.GL_FRONT_AND_BACK, Gl.GL_FILL);
           
            foreach (var sprite in Children)
            {
                sprite.Draw();
            }

            Gl.glDisable(Gl.GL_TEXTURE_2D);
            Gl.glDisable(Gl.GL_BLEND);
            Gl.glEnable(Gl.GL_DEPTH_TEST);
        }

        private void SetupProjection()
        {
            int[] viewport = new int[4];
            Gl.glGetIntegerv(Gl.GL_VIEWPORT, viewport);

            Gl.glMatrixMode(Gl.GL_PROJECTION);
            Gl.glLoadIdentity();
            Glu.gluOrtho2D(0.0, viewport[2], 0.0, viewport[3]);
            Gl.glMatrixMode(Gl.GL_MODELVIEW);
            Gl.glLoadIdentity();

            Size = new SizeF(viewport[2], viewport[3]);

            Gl.glTranslatef(Size.Width / 2, Size.Height / 2, 0);
        }

        private List<Sprite> m_activeSprites = new List<Sprite>();

        /// <summary>
        /// Gets the Pyxis.UI.Layers.Sprites.Sprite under the cursor.
        /// </summary>
        public List<Sprite> ActiveSprites
        {
            get
            {
                return m_activeSprites;
            }
        }

        #region handle mouse events

        /// <summary>
        /// Represents the method that will handle the MouseMove event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="args">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public bool HandleMouseMove(MouseEventArgs args)
        {
            m_activeSprites = GetSpritesFromScreenLocation(args.Location);
            return HandleMouseEvent(args, new PointF(args.X - Size.Width / 2, args.Y - Size.Height / 2), sprite => sprite.MouseMove);
        }

        /// <summary>
        /// Represents the method that will handle the MouseDown event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="args">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public bool HandleMouseDown(MouseEventArgs args)
        {
            return HandleMouseEvent(args, new PointF(args.X - Size.Width / 2, args.Y - Size.Height / 2), sprite =>
            {
                if (sprite == null)
                {
                    throw new ArgumentNullException("sprite");
                }
                return sprite.MouseDown;
            });
        }

        /// <summary>
        /// Represents the method that will handle the MouseUp event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="args">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public bool HandleMouseUp(MouseEventArgs args)
        {
            return HandleMouseEvent(args, new PointF(args.X - Size.Width / 2, args.Y - Size.Height / 2), sprite =>
            {
                if (sprite == null)
                {
                    throw new ArgumentNullException("sprite");
                }
                return sprite.MouseUp;
            });
        }

        /// <summary>
        /// Represents the method that will handle the MouseClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="args">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public bool HandleMouseClick(MouseEventArgs args)
        {
            return HandleMouseEvent(args, new PointF(args.X - Size.Width / 2, args.Y - Size.Height / 2), sprite =>
            {
                if (sprite == null)
                {
                    throw new ArgumentNullException("sprite");
                }
                return sprite.MouseClick;
            });
        }

        /// <summary>
        /// Represents the method that will handle the MouseDoubleClick event of the Pyxis.UI.PyxisView control.
        /// </summary>
        /// <param name="args">A MouseEventArgs that contains the event data.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        public bool HandleMouseDoubleClick(MouseEventArgs args)
        {
            return HandleMouseEvent(args, new PointF(args.X - Size.Width / 2, args.Y - Size.Height / 2), sprite =>
            {
                if (sprite == null)
                {
                    throw new ArgumentNullException("sprite");
                }
                return sprite.MouseDoubleClick;
            });
        }

        /// <summary>
        /// Get all the Pyxis.UI.Layers.Sprites.Sprite at a specific screen location.
        /// </summary>
        /// <param name="location">A System.Drawing.Point representing the location of interest on the screen.</param>
        /// <returns>The Pyxis.UI.Layers.Sprites.Sprite at <paramref name="location"/>.</returns>
        public List<Sprite> GetSpritesFromScreenLocation(Point location)
        {
            return GetSpritesFromScreenLocation(new PointF(location.X - Size.Width / 2, location.Y - Size.Height / 2)).ToList();
        }

        #endregion

        #region Texture resource handlers

        private readonly HashSet<int> m_textureHandles = new HashSet<int>();
        private readonly HashSet<int> m_textureHandlesToRemove = new HashSet<int>();

        /// <summary>
        /// Dispose of the resources managed by the Pyxis.UI.Layers.Sprites.RootSprite.
        /// </summary>
        public void HandleDispose()
        {
            var handles = m_textureHandles.ToArray();
            Gl.glDeleteTextures(handles.Length, handles);
            m_textureHandles.Clear();
        }

        /// <summary>
        /// Allocate a texture handle in the graphics library.
        /// </summary>
        /// <returns>The handle to the texture.</returns>
        public int AllocateTextureHandle()
        {
            int[] handles = new int[1];
            Gl.glGenTextures(1, handles);
            m_textureHandles.Add(handles[0]);
            return handles[0];            
        }

        /// <summary>
        /// Mark a texture handle in the graphics library for release.
        /// </summary>
        /// <param name="handle">The handle to the texture.</param>
        public void ReleaseTextureHandle(int handle)
        {
            lock (m_textureHandlesToRemove)
            {
                m_textureHandlesToRemove.Add(handle);
            }
        }

        /// <summary>
        /// this function gets called under the right OpenGL context before paint is done
        /// </summary>
        private void ReleaseTexturesIfNeeded()
        {
            lock (m_textureHandlesToRemove)
            {
                if (m_textureHandlesToRemove.Count > 0)
                {
                    foreach (var handle in m_textureHandlesToRemove)
                    {
                        if (m_textureHandles.Contains(handle))
                        {
                            var handles = new[] { handle };
                            Gl.glDeleteTextures(1, handles);
                            m_textureHandles.Remove(handle);
                        }
                    }
                    m_textureHandlesToRemove.Clear();
                }
            }
        }

        #endregion
    }
}
