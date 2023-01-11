using System;
using System.Drawing;
using System.Drawing.Imaging;
using Tao.OpenGl;

namespace Pyxis.UI.Layers.Sprites
{
    /// <summary>
    /// Exposes functions for interacting with a graphics library context.
    /// </summary>
    public interface IGlContext
    {
        /// <summary>
        /// Allocate a handle to a texture in the graphic library context.
        /// </summary>
        /// <returns>The handle to the allocated texture.</returns>
        int AllocateTextureHandle();
        /// <summary>
        /// Release a handle to a texture in the graphic library context.
        /// </summary>
        /// <param name="handle">Handle to the texture to release.</param>
        void ReleaseTextureHandle(int handle);
    }

    /// <summary>
    /// Encapsulates a texture in a graphics library context.
    /// </summary>
    public class Texture : IDisposable
    {
        private readonly IGlContext m_context;
        private int m_handle;
        private int m_width;
        private int m_height;

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Sprites.Texture class from a System.Drawing.Bitmap.
        /// </summary>
        /// <param name="context">The graphics library context to use.</param>
        /// <param name="bitmap">The System.Drawing.Bitmap to initialize the texture.</param>
        public Texture(IGlContext context, Bitmap bitmap)
        {
            m_context = context;
            m_handle = m_context.AllocateTextureHandle();

            var bitmapData = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);
            Gl.glBindTexture(Gl.GL_TEXTURE_2D, m_handle);
            Gl.glTexImage2D(Gl.GL_TEXTURE_2D, 0, Gl.GL_RGBA, bitmap.Width, bitmap.Height, 0, Gl.GL_BGRA, Gl.GL_UNSIGNED_BYTE, bitmapData.Scan0);
            Gl.glTexParameteri(Gl.GL_TEXTURE_2D, Gl.GL_TEXTURE_MIN_FILTER, Gl.GL_LINEAR);
            Gl.glTexParameteri(Gl.GL_TEXTURE_2D, Gl.GL_TEXTURE_MAG_FILTER, Gl.GL_LINEAR);
            Gl.glTexParameteri(Gl.GL_TEXTURE_2D, Gl.GL_TEXTURE_WRAP_S, Gl.GL_CLAMP_TO_EDGE);
            Gl.glTexParameteri(Gl.GL_TEXTURE_2D, Gl.GL_TEXTURE_WRAP_T, Gl.GL_CLAMP_TO_EDGE);
            Gl.glBindTexture(Gl.GL_TEXTURE_2D, 0);
            bitmap.UnlockBits(bitmapData);

            m_width = bitmap.Width;
            m_height = bitmap.Height;
        }

        /// <summary>
        /// Get the graphics library handle.
        /// </summary>
        public int Handle { get { return m_handle; } }
        /// <summary>
        /// Get the width of the Pyxis.UI.Layers.Sprites.Texture.
        /// </summary>
        public int Width { get { return m_width; } }
        /// <summary>
        /// Get the height of the Pyxis.UI.Layers.Sprites.Texture.
        /// </summary>
        public int Height { get { return m_height; } }

        /// <summary>
        /// Bind the Pyxis.UI.Layers.Sprites.Texture to a graphics library target. 
        /// </summary>
        public void Bind()
        {
            Gl.glBindTexture(Gl.GL_TEXTURE_2D, m_handle);
        }

        /// <summary>
        /// Unbind the Pyxis.UI.Layers.Sprites.Texture from a graphics library target.
        /// </summary>
        public void Unbind()
        {
            Gl.glBindTexture(Gl.GL_TEXTURE_2D, 0);
        }

        /// <summary>
        /// Releases all resources used by the Pyxis.UI.Layers.Sprites.Texture.
        /// </summary>
        public void Dispose()
        {
            if (m_handle != -1)
            {
                m_context.ReleaseTextureHandle(m_handle);
                m_handle = -1;
            }
        }
    }
}
