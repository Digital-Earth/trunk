using Awesomium.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Pyxis.WorldView.Studio.Layers.Html
{
    /// <summary>
    /// OpenGLSurface implements Awesomium.Core.ISurface based on BitmapSurface.
    /// 
    /// BitmapSurface keep a RGBA bitmap of the current view. Which is great of copying 
    /// the bitmap into an OpenGLTexture. 
    /// However, BitmapSurface doesn't keep track of the modified texture. 
    /// Therefore, OpenGLSurface is basically a proxy to BitmapSurface instant that keep
    /// Record of the modified area which can be accessed by ModifiedRect.
    /// 
    /// The user of OpenGLSurface can call ClearModifiedRect to reset the tracking.
    /// </summary>
    internal class OpenGLSurface : ISurface
    {
        public BitmapSurface BitmapSurface;
        public AweRect ModifiedRect = AweRect.Empty;

        public void Initialize(IWebView view, int width, int height)
        {
            BitmapSurface = new BitmapSurface(width,height);            
        }

        public void Paint(IntPtr srcBuffer, int srcRowSpan, AweRect srcRect, AweRect destRect)
        {
            AddInvalidRect(destRect);
            (BitmapSurface as Awesomium.Core.ISurface).Paint(srcBuffer, srcRowSpan, srcRect, destRect);
        }

        public void Scroll(int dx, int dy, AweRect clipRect)
        {
            AddInvalidRect(clipRect);
            (BitmapSurface as ISurface).Scroll(dx, dy, clipRect);
        }
        
        public void Dispose()
        {
            (BitmapSurface as ISurface).Dispose();
        }

        public void AddInvalidRect(Awesomium.Core.AweRect newRect)
        {
            if (ModifiedRect == AweRect.Empty)
            {
                ModifiedRect = newRect;
            }
            else
            {
                var minX = Math.Min(newRect.X, ModifiedRect.X);
                var maxX = Math.Max(newRect.X + newRect.Width, ModifiedRect.X + ModifiedRect.Width);
                var minY = Math.Min(newRect.Y, ModifiedRect.Y);
                var maxY = Math.Max(newRect.Y + newRect.Height, ModifiedRect.Y + ModifiedRect.Height);

                ModifiedRect = new AweRect(minX, minY, maxX - minX, maxY - minY);
            }
        }

        public void ClearModifiedRect()
        {
            ModifiedRect = AweRect.Empty;
        }

        public bool WasModified
        {
            get
            {
                return ModifiedRect != AweRect.Empty;
            }
        }
    }
}
