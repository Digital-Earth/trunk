//
//      FILE:   DrawingUtilities.cs.
//
//    AUTHOR:   Grant Frisken
//
// COPYRIGHT:   Copyright 2004 
//              Infralution
//              6 Bruce St 
//              Mitcham Australia
//
using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Runtime.InteropServices;
using System.Drawing.Imaging;
using System.Windows.Forms;
using System.Windows.Forms.VisualStyles;
using System.Diagnostics;
using System.Globalization;
using Infralution.Common;
namespace Infralution.Controls
{
    /// <summary>
    /// Provides utilities for drawing.
    /// </summary>
    public static class DrawingUtilities
    {

        #region API Calls

        [StructLayout(LayoutKind.Sequential)]
        private struct RECT
        {
            public int left;
            public int top;
            public int right;
            public int bottom;

            public RECT(Rectangle rc)
            {
                left = rc.X;
                top = rc.Y;
                right = rc.Right - 1;
                bottom = rc.Bottom - 1;
            }
        }

        [StructLayout(LayoutKind.Sequential)]
        private struct ICONINFO
        {
            public bool fIcon;         
            public Int32 xHotspot;     
            public Int32 yHotspot;     
            public IntPtr hbmMask;    
            public IntPtr hbmColor;    
        }

        [DllImport("user32.dll")]
        private static extern bool GetIconInfo(IntPtr hIcon, out ICONINFO piconinfo);

        [DllImport("user32.dll")]
        private static extern IntPtr CreateIconIndirect([In] ref ICONINFO piconinfo);


        [DllImport("uxtheme", CharSet = CharSet.Unicode)]
        private static extern int DrawThemeBackground(
            IntPtr hTheme,
            IntPtr hdc,
            int iPartId,
            int iStateId,
            ref RECT pRect,
            ref RECT pClipRect);

        [DllImport("user32.dll")]
        private static extern IntPtr GetWindowDC(IntPtr hWnd);

        [DllImport("gdi32.dll")]
        private static extern int ExcludeClipRect(IntPtr hdc, int nLeftRect, int nTopRect,
                                                  int nRightRect, int nBottomRect);

        [DllImport("user32.dll")]
        private static extern int ReleaseDC(IntPtr hWnd, IntPtr hDC);

        [DllImport("user32.dll")]
        static extern bool GetWindowRect(IntPtr hWnd, out RECT lpRect);

        [Flags()]
        private enum Layout : uint
        {
            RTL = 1,
            LTR = 0,
            BitmapOrientationPreserved = 8
        }

        [DllImport("gdi32.dll")]
        private extern static uint SetLayout(IntPtr hdc, Layout dwLayout);

        [DllImport("gdi32.dll")]
        private static extern IntPtr CreateRectRgn(int nLeftRect, int nTopRect, 
                                                   int nRightRect, int nBottomRect);

        [DllImport("gdi32.dll")]
        private static extern int SelectClipRgn(IntPtr hdc, IntPtr hrgn);

        [DllImport("gdi32.dll")]
        private static extern bool DeleteObject(IntPtr hObject);

        [DllImport("user32.DLL")] 
        private static extern int SendMessage(IntPtr hWnd, int Msg, IntPtr wParam, int lParam); 

        [DllImport("gdi32.dll")]
        static extern bool SetWindowOrgEx(IntPtr hdc, int X, int Y, System.IntPtr lpPoint);

        [DllImport("gdi32.dll")]
        static extern int SaveDC(IntPtr hdc);

        [DllImport("gdi32.dll")]
        static extern bool RestoreDC(IntPtr hdc, int nSavedDC);

        [DllImport("user32.dll")]
        static extern bool DrawIconEx(IntPtr hdc, int xLeft, int yTop, IntPtr hIcon,
            int cxWidth, int cyHeight, int istepIfAniCur, IntPtr hbrFlickerFreeDraw,
            int diFlags);

        [DllImport("user32.dll")]
        private static extern bool DestroyIcon(IntPtr hIcon);

        #endregion

        #region Public Interface

        /// <summary>
        /// Return a client x coordinate adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="control">The control to translate for</param>
        /// <param name="x">The coordinate to adjust</param>
        /// <returns>The adjusted coordinate</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the coordinate to give right to left
        /// layout
        /// </remarks>
        static public int RtlTranslateX(Control control, int x)
        {
            if (control.RightToLeft == RightToLeft.Yes)
            {
                return control.ClientSize.Width - x - 1;
            }
            return x;
        }

        /// <summary>
        /// Return a point in client coordinates adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="control">The control to translate for</param>
        /// <param name="p">The point to adjust</param>
        /// <returns>The adjusted point</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the point to give right to left
        /// layout
        /// </remarks>
        static public Point RtlTranslatePoint(Control control, Point p)
        {
            p.X = RtlTranslateX(control, p.X);
            return p;
        }

        /// <summary>
        /// Return a rectangle in client coordinates adjusted for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="control">The control to translate for</param>
        /// <param name="r">The rectangle to adjust</param>
        /// <returns>The adjusted rectangle</returns>
        /// <remarks>
        /// If <see cref="RightToLeft"/> is set then this adjusts the rectangle to give right to left
        /// layout
        /// </remarks>
        static public Rectangle RtlTranslateRect(Control control, Rectangle r)
        {
            if (control.RightToLeft == RightToLeft.Yes)
            {
                int x = control.ClientSize.Width - r.X - r.Width;
                return new Rectangle(x, r.Y, r.Width, r.Height);
            }            
            return r;
        }

        /// <summary>
        /// Translate content alignment to account for <see cref="RightToLeft"/> positioning.
        /// </summary>
        /// <param name="control">The control containing the element to be aligned</param>
        /// <param name="alignment">The alignment to translate</param>
        /// <returns>The adjusted alignment value</returns>
        static public System.Drawing.ContentAlignment RtlTranslateAlignment(Control control, System.Drawing.ContentAlignment alignment)
        {
            if (control.RightToLeft == RightToLeft.Yes)
            {
                switch (alignment)
                {
                    case System.Drawing.ContentAlignment.TopLeft:
                        alignment = System.Drawing.ContentAlignment.TopRight;
                        break;
                    case System.Drawing.ContentAlignment.TopRight:
                        alignment = System.Drawing.ContentAlignment.TopLeft;
                        break;
                    case System.Drawing.ContentAlignment.MiddleLeft:
                        alignment = System.Drawing.ContentAlignment.MiddleRight;
                        break;
                    case System.Drawing.ContentAlignment.MiddleRight:
                        alignment = System.Drawing.ContentAlignment.MiddleLeft;
                        break;
                    case System.Drawing.ContentAlignment.BottomLeft:
                        alignment = System.Drawing.ContentAlignment.BottomRight;
                        break;
                    case System.Drawing.ContentAlignment.BottomRight:
                        alignment = System.Drawing.ContentAlignment.BottomLeft;
                        break;
                }
            }
            return alignment;
        }

        /// <summary>
        /// Set the clipping rectangle for the given graphics context.  
        /// </summary>
        /// <remarks>
        /// Graphics.SetClip does not set the clipping rectangle of the associated GDI context and so 
        /// graphics operations which use GDI (such as DrawIcon) are not clipped properly.
        /// </remarks>
        /// <param name="graphics">The context to set the clipping rectangle for</param>
        /// <param name="rect">The rectangle to use as a clipping rectangle</param>
        static public void SetClip(Graphics graphics, Rectangle rect)
        {
            graphics.SetClip(rect);
            Rectangle deviceRect = TransformRect(graphics, CoordinateSpace.Device, CoordinateSpace.World, rect);
            IntPtr dc = graphics.GetHdc();
            IntPtr rgn = CreateRectRgn(deviceRect.X, deviceRect.Y, deviceRect.Right, deviceRect.Bottom);
            int result = SelectClipRgn(dc, rgn);
            DeleteObject(rgn);
            graphics.ReleaseHdc(dc);
        }

        /// <summary>
        /// Reset the current clip region for the given graphics context
        /// </summary>
        /// <remarks>
        /// Resets the clipping region if it has been set using <see cref="SetClip"/>
        /// </remarks>
        /// <param name="graphics">The context to reset the clipping region for</param>
        static public void ResetClip(Graphics graphics)
        {
            graphics.ResetClip();
            IntPtr dc = graphics.GetHdc();
            SelectClipRgn(dc, IntPtr.Zero);
            graphics.ReleaseHdc(dc);           
        }

        /// <summary>
        /// Get the current clip bounds for the given context
        /// </summary>
        /// <param name="graphics">The context to get the clip bounds for</param>
        /// <returns>A rectangle bounding the current clip region</returns>
        static public Rectangle GetClip(Graphics graphics)
        {
            RectangleF rectf = graphics.ClipBounds;
            return new Rectangle((int)rectf.X, (int) rectf.Y, 
                                 (int)rectf.Width, (int)rectf.Height);
        }
 
        /// <summary>
        /// Are cursors larger than 32x32 supported by the operating system
        /// </summary>
        /// <returns></returns>
        static public bool LargeCursorsSupported
        {   
            get
            {
                OperatingSystem os = Environment.OSVersion;
                return (os.Platform == PlatformID.Win32NT) && (os.Version.Major >= 5);
            }
        }

        /// <summary>
        /// Create a translucent cursor from the given image.
        /// </summary>
        /// <remarks>
        /// You must call <see cref="DestroyCursor"/> to dispose of the cursor fully.
        /// </remarks>
        /// <param name="image">The image to create the cursor from</param>
        /// <param name="transparency">How transparent the cursor should be (0 = Invisible, 1.0 = Opaque)</param>
        /// <param name="hotSpotX">The location of the hotspot relative to the left of the image</param>
        /// <param name="hotSpotY">The location of the hotspot relative to the top of the image</param>
        /// <param name="overlayCursor">The cursor to overlay the image (may be null)</param>
        /// <param name="overlayOffsetX">The X offset of the image relative to the overlay cursor</param>
        /// <param name="overlayOffsetY">The Y offset of the image relative to the overlay cursor</param>
        /// <param name="rightToLeft">Should the overlay cursor be flipped and displayed at the right</param>
        /// <returns>A translucent cursor</returns>
        static public Cursor CreateCursor(Image image, float transparency,
                                          int hotSpotX, int hotSpotY,
                                          Cursor overlayCursor,
                                          int overlayOffsetX, int overlayOffsetY,
                                          RightToLeft rightToLeft)
        {
            int width = image.Width;
            int height = image.Height;

            int bwidth = width + overlayOffsetX;
            int bheight = height + overlayOffsetY;

            if ((bwidth > 32 || bheight > 32) && !LargeCursorsSupported)
                throw new ArgumentOutOfRangeException("Large Cursors not supported on this OS");

            using (Bitmap bitmap = new Bitmap(bwidth, bheight))
            {
                using (Graphics graphics = Graphics.FromImage(bitmap))
                {
                    graphics.Clear(Color.Transparent);
                    ImageAttributes ia = new ImageAttributes();
                    ColorMatrix cm = new ColorMatrix();
                    cm.Matrix33 = transparency;
                    ia.SetColorMatrix(cm);

                    Rectangle destRect = new Rectangle(overlayOffsetX, overlayOffsetX, width, height);
                    if (rightToLeft == RightToLeft.Yes)
                    {
                        destRect.X = 0;
                        hotSpotX = overlayOffsetX + width - hotSpotX;
                    }
                    graphics.DrawImage(image, destRect, 0, 0, width, height, GraphicsUnit.Pixel, ia);

                    if (overlayCursor != null)
                    {
                        Size size = overlayCursor.Size;
                        Rectangle cursorRect = new Rectangle(0, 0, size.Width, size.Height);
                        if (rightToLeft == RightToLeft.Yes)
                        {
                            cursorRect.X = width + overlayOffsetX - 1;
                            cursorRect.Width = -cursorRect.Width;
                        }
                        overlayCursor.DrawStretched(graphics, cursorRect);
                    }
                }

                IntPtr hIcon = bitmap.GetHicon();
                ICONINFO iconInfo;
                GetIconInfo(hIcon, out iconInfo);
                iconInfo.fIcon = false;
                iconInfo.xHotspot = hotSpotX;
                iconInfo.yHotspot = hotSpotY;
                IntPtr hCursor = CreateIconIndirect(ref iconInfo);
                DeleteObject(hIcon);
                return new Cursor(hCursor);
            }
        }

        /// <summary>
        /// Destroy a cursor created using <see cref="CreateCursor"/>
        /// </summary>
        /// <remarks>
        /// This method must be used to destroy the underlying icon for the cursor.
        /// </remarks>
        /// <param name="cursor">The cursor to destroy</param>
        static public void DestroyCursor(Cursor cursor)
        {
            DestroyIcon(cursor.Handle);
            cursor.Dispose();
        }

        /// <summary>
        /// Transform the given point from one graphics coordinate space to another
        /// </summary>
        /// <param name="graphics">The context to apply the transform in</param>
        /// <param name="destSpace">The space to transform to</param>
        /// <param name="srcSpace">The space to transform from</param>
        /// <param name="point">The point to transform</param>
        /// <returns>The transformed point in the destination space</returns>
        static public Point TransformPoint(Graphics graphics, 
                                           CoordinateSpace destSpace, 
                                           CoordinateSpace srcSpace, 
                                           Point point)
        {
            Point[] points = { point };
            graphics.TransformPoints(destSpace, srcSpace, points);
            return points[0];
        }

        /// <summary>
        /// Transform the given rectangle from one graphics coordinate space to another
        /// </summary>
        /// <param name="graphics">The context to apply the transform in</param>
        /// <param name="destSpace">The space to transform to</param>
        /// <param name="srcSpace">The space to transform from</param>
        /// <param name="rect">The rectangle to transform</param>
        /// <returns>The transformed rectangle in the destination space</returns>
        static public Rectangle TransformRect(Graphics graphics, 
                                              CoordinateSpace destSpace, 
                                              CoordinateSpace srcSpace, 
                                              Rectangle rect)
        {
            Point[] points = { rect.Location,  new Point(rect.Right, rect.Bottom) };
            graphics.TransformPoints(destSpace, srcSpace, points);
            rect.Location = points[0];
            rect.Width = points[1].X - points[0].X + 1;
            rect.Height = points[1].Y - points[0].Y + 1;
            return rect;
        }

        /// <summary>
        /// Draw an icon to the given graphics context
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="icon">The icon to draw</param>
        /// <param name="x">The x location</param>
        /// <param name="y">The y location</param>
        /// <param name="printing">Is it a printing context</param>
        /// <param name="mirror">Should the icon be drawn mirrored</param>
        /// <remarks>
        /// The standard Graphics.DrawIcon method does not handle drawing to printer
        /// contexts properly.  The performance of Graphics.DrawIcon is also about 100 times worse 
        /// under .NET 2.0 framework then it was under .NET 1.1.  This method addresses both these
        /// shortcomings.  If the printing flag is set to true then it converts the icon to a bitmap 
        /// and uses Graphics.DrawImage otherwise it uses the DrawIconEx API to draw the icon.
        /// </remarks>
        static public void DrawIcon(Graphics graphics, Icon icon, int x, int y, bool printing, bool mirror)
        {
            int width = icon.Width;
            if (mirror)
            {
                x += width - 1;
                width = -width;
            }

            if (printing)
            {
                // don't use icon.ToBitmap because it has problems with transparency with
                // 256 color icons
                //
                using (Bitmap bitmap = Bitmap.FromHicon(icon.Handle))
                {
                    graphics.DrawImage(bitmap, x, y, width, bitmap.Height);
                }
            }
            else
            {
                IntPtr hDC = graphics.GetHdc();
                DrawIconEx(hDC, x, y, icon.Handle, width, icon.Height, 0, IntPtr.Zero, 3);
                graphics.ReleaseHdc(hDC);
            }
        }

        /// <summary>
        /// Draw an icon to the given graphics context
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="icon">The icon to draw</param>
        /// <param name="x">The x location</param>
        /// <param name="y">The y location</param>
        /// <param name="printing">Is it a printing context</param>
        static public void DrawIcon(Graphics graphics, Icon icon, int x, int y, bool printing)
        {
            DrawIcon(graphics, icon, x, y, printing, false);
        }

        /// <summary>
        /// Draw an icon to the given graphics context
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="icon">The icon to draw</param>
        /// <param name="x">The x location</param>
        /// <param name="y">The y location</param>
        static public void DrawIcon(Graphics graphics, Icon icon, int x, int y)
        {
            DrawIcon(graphics, icon, x, y, false, false);
        }

        /// <summary>
        /// Draw an icon to the given graphics context
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="icon">The icon to draw</param>
        /// <param name="x">The x location</param>
        /// <param name="y">The y location</param>
        /// <param name="alignment">The alignment for the icon</param>
        /// <param name="printing">Is it a printing context</param>
        /// <param name="mirror">Should the icon be drawn mirrored</param>
        static public void DrawIcon(Graphics graphics, Icon icon, int x, int y, 
                                    System.Drawing.ContentAlignment alignment, 
                                    bool printing, bool mirror)
        {
            switch (alignment)
            {
                case System.Drawing.ContentAlignment.BottomCenter:
                    y -= icon.Height;
                    x -= icon.Width / 2;
                    break;
                case System.Drawing.ContentAlignment.BottomLeft:
                    y -= icon.Height;
                    break;
                case System.Drawing.ContentAlignment.BottomRight:
                    y -= icon.Height;
                    x -= icon.Width;
                    break;
                case System.Drawing.ContentAlignment.MiddleCenter:
                    y -= icon.Height / 2;
                    x -= icon.Width / 2;
                    break;
                case System.Drawing.ContentAlignment.MiddleLeft:
                    y -= icon.Height / 2;
                    break;
                case System.Drawing.ContentAlignment.MiddleRight:
                    y -= icon.Height / 2;
                    x -= icon.Width;
                    break;
                case System.Drawing.ContentAlignment.TopCenter:
                    x -= icon.Width / 2;
                    break;
                case System.Drawing.ContentAlignment.TopLeft:
                    break;
                case System.Drawing.ContentAlignment.TopRight:
                    x -= icon.Width;
                    break;
            }
            DrawIcon(graphics, icon, x, y, printing, mirror);
        }

        /// <summary>
        /// Draw an image repeatedly to fill the given rectangle
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="image">The image to tile</param>
        /// <param name="rect">The rectangle to draw to</param>
        static public void TileImage(Graphics graphics, Image image, Rectangle rect)
        {
            Region oldClip = graphics.Clip;
            graphics.SetClip(rect);           
            graphics.InterpolationMode = InterpolationMode.NearestNeighbor;
            for (int x = rect.Left; x < rect.Right; x += image.Width)
            {
                for (int y = rect.Top; y < rect.Bottom; y += image.Height)
                {
                    graphics.DrawImage(image, x, y, image.Width, image.Height);
                }
            }
            graphics.SetClip(oldClip, CombineMode.Replace);
        }

        /// <summary>
        /// Stretch an image to fill the given rectangle while maintain aspect ratio of the image
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="image">The image to draw</param>
        /// <param name="rect">The rectangle to draw to</param>
        static public void StretchImageSymmetric(Graphics graphics, Image image, Rectangle rect)
        {
            float imageAR = (float)image.Width / (float)image.Height;
            float rectAR = (float)rect.Width / (float)rect.Height;
            Rectangle target = rect;
            
            if (imageAR > rectAR)
            {
                target.Height = (int)((float)rect.Width / imageAR);
                target.Y = rect.Y + (rect.Height - target.Height) / 2;
            }
            else
            {
                target.Width = (int)((float)rect.Height * imageAR);
                target.X = rect.X + (rect.Width - target.Width) / 2;
            }
            graphics.DrawImage(image, target);
        }

        /// <summary>
        /// Draw an image to a rectangle
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="image">The image to draw</param>
        /// <param name="rect">The rectangle to fill</param>
        /// <param name="mode">Determines how the image is scaled to fill the rectangle</param>
        static public void DrawImage(Graphics graphics, Image image, Rectangle rect, ImageDrawMode mode)
        {
            switch (mode)
            {
                case ImageDrawMode.Tile:
                    TileImage(graphics, image, rect);
                    break;
                case ImageDrawMode.Stretch:
                    graphics.DrawImage(image, rect);
                    break;
                case ImageDrawMode.StretchSymmetric:
                    StretchImageSymmetric(graphics, image, rect);
                    break;
            }
        }

        /// <summary>
        /// Paint the given control to a graphics context
        /// </summary>
        /// <remarks>
        /// <see cref="Control.InvokePaint"/> only works properly for those controls that do
        /// their painting in managed code.  If the control uses an unmanaged control (such as
        /// as a textbox) as the basis then InvokePaint does not work.  This method works by sending
        /// a WM_PRINT message to the control which works for both managed and unmanaged based
        /// controls.
        /// </remarks>
        /// <param name="control">The control to paint</param>
        /// <param name="graphics">The context to paint to</param>
        /// <param name="x">The x location to paint to</param>
        /// <param name="y">The y location to paint to</param>
        static public void PaintControl(Control control, Graphics graphics, int x, int y)
        {
            const int WM_PRINT = 0x317;
            const int PRF_CLIENT = 0x4;
            const int PRF_CHILDREN = 0x10;
            const int PRF_NON_CLIENT = 0x2; 

            IntPtr hWnd = control.Handle; 
            IntPtr hDC = graphics.GetHdc(); 
            SaveDC(hDC);
            SetWindowOrgEx(hDC, -x, -y, IntPtr.Zero);
            SendMessage(hWnd, WM_PRINT, hDC, PRF_CLIENT | PRF_CHILDREN | PRF_NON_CLIENT);
            RestoreDC(hDC, -1);
            graphics.ReleaseHdc(hDC);
        }

        /// <summary>
        /// Draw a XP Themed border for the given control
        /// </summary>
        /// <param name="control">The control to paint the border for</param>
        /// <remarks>
        /// This method is typically called to paint the non-client area of a control.
        /// </remarks>
        static public void DrawThemeBorder(Control control)
        {
            VisualStyleRenderer renderer = new VisualStyleRenderer(VisualStyleElement.TextBox.TextEdit.Normal);

            // NB: we have to use GetWindowDC for drawing to Non-Client areas.  
            // The graphics object returned by control.CreateGraphics will not
            // work for non-client areas
            //
            IntPtr hDC = GetWindowDC(control.Handle);

            // get the window rectangle rather than simply using the control bounds
            // because at the time the NC area is painted the bounds of the control
            // may not have been updated yet
            //
            RECT wb;
            GetWindowRect(control.Handle, out wb);
            int width = wb.right - wb.left;
            int height = wb.bottom - wb.top;
            using (Graphics graphics = Graphics.FromHdc(hDC))
            {
                Rectangle bounds = new Rectangle(0, 0, width, height);
                ExcludeClipRect(hDC, 1, 1, width - 1, height - 1);
                renderer.DrawBackground(graphics, bounds);
            }
            ReleaseDC(control.Handle, hDC);
        }

        /// <summary>
        /// Draws a themed background in the given control with mirroring for
        /// right to left layouts if required
        /// </summary>
        /// <param name="control">The control that owns the context</param>
        /// <param name="graphics">Graphics context to draw to</param>
        /// <param name="renderer">The visual styles renderer to use</param>
        /// <param name="bounds">The bounds to draw to</param>
        /// <param name="rtl">Is the layout right to left</param>
        static public void DrawThemeBackground(Control control,
                                               Graphics graphics,
                                               VisualStyleRenderer renderer,
                                               Rectangle bounds,
                                               RightToLeft rtl)
        {
            // NB: We make the API call to DrawThemeBackground ourselves rather than calling
            // renderer.DrawBackground because renderer.DrawBackground applies the GDI+ clipping areas
            // to the DC which screws up the mirrored Layout.  Our implementation is also somewhat
            // more efficient
            //
            Rectangle deviceBounds = DrawingUtilities.TransformRect(graphics, CoordinateSpace.Device, CoordinateSpace.World, bounds); ;

            IntPtr hDC = graphics.GetHdc();
            if (rtl == RightToLeft.Yes)
            {
                SetLayout(hDC, Layout.RTL);
                deviceBounds.X = control.ClientSize.Width - deviceBounds.X - deviceBounds.Width;
                RECT rc = new RECT(deviceBounds);
                DrawThemeBackground(renderer.Handle, hDC, renderer.Part, renderer.State, ref rc, ref rc);
                SetLayout(hDC, Layout.LTR);
            }
            else
            {
                RECT rc = new RECT(deviceBounds);
                DrawThemeBackground(renderer.Handle, hDC, renderer.Part, renderer.State, ref rc, ref rc);
            }
            graphics.ReleaseHdc(hDC);
        }

        /// <summary>
        /// Draw a rounded rectangle with the given pen
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="pen">The pen to use</param>
        /// <param name="rect">The bounds of the rectangle</param>
        /// <param name="radius">The rounding radius</param>
        static public void DrawRoundedRect(Graphics graphics, Pen pen, Rectangle rect, int radius)
        {
            using (GraphicsPath path = CreateRoundedRectPath(rect, radius))
            {
                // set the smoothing mode to antialize rounded rectangles
                //
                SmoothingMode smoothingMode = graphics.SmoothingMode;
                graphics.SmoothingMode = SmoothingMode.AntiAlias;
                graphics.DrawPath(pen, path);
                graphics.SmoothingMode = smoothingMode;
            }
        }

        /// <summary>
        /// Fill a rounded rectangle with the given brush and using thge given pen for the border
        /// </summary>
        /// <param name="graphics">The context to draw to</param>
        /// <param name="brush">The brush to use</param>
        /// <param name="pen">The pen (if any) to use</param>
        /// <param name="rect">The bounds of the rectangle</param>
        /// <param name="radius">The rounding radius</param>
        static public void FillRoundedRect(Graphics graphics, Brush brush, Pen pen, Rectangle rect, int radius)
        {
            using (GraphicsPath path = CreateRoundedRectPath(rect, radius))
            {
                SmoothingMode smoothingMode = graphics.SmoothingMode;
                graphics.SmoothingMode = SmoothingMode.AntiAlias;
                graphics.FillPath(brush, path);
                if (pen != null)
                {
                    graphics.DrawPath(pen, path);
                }
                graphics.SmoothingMode = smoothingMode;
            }
        }

        #endregion

        #region Local Methods

        /// <summary>
        /// Create a graphics path for drawing/filling rounded rectangles
        /// </summary>
        /// <param name="rect">The rectangle bounds</param>
        /// <param name="radius">The rounding radius</param>
        /// <returns>A graphics path</returns>
        static private GraphicsPath CreateRoundedRectPath(Rectangle rect, int radius)
        {
            GraphicsPath path = new GraphicsPath();
            path.AddLine(rect.X + radius, rect.Y, rect.Right - (radius * 2), rect.Y);
            path.AddArc(rect.Right - (radius * 2), rect.Y, radius * 2, radius * 2, 270, 90);
            path.AddLine(rect.Right, rect.Y + radius, rect.Right, rect.Bottom - (radius * 2));
            path.AddArc(rect.Right - (radius * 2), rect.Bottom - (radius * 2), radius * 2, radius * 2, 0, 90);
            path.AddLine(rect.Right - (radius * 2), rect.Bottom, rect.X + radius, rect.Bottom);
            path.AddArc(rect.X, rect.Bottom - (radius * 2), radius * 2, radius * 2, 90, 90);
            path.AddLine(rect.X, rect.Bottom - (radius * 2), rect.X, rect.Y + radius);
            path.AddArc(rect.X, rect.Y, radius * 2, radius * 2, 180, 90);
            path.CloseFigure();
            return path;
        }

        #endregion
    }
}
