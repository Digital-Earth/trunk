using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;
using System.Xml;
using System.Net;
using System.Linq;


namespace ApplicationUtility
{
    public class ManagedBitmapServer : BitmapServerProvider, IDirectorReferenceCounter
    {
        #region Static memebers

        public class DemoIconsInfo
        {
            public const string ResourceName = "demo-icons2.png";
            public const int IconWidth = 64;
            public const int IconHeight = 64;            
            public const int IconPaletteWidth = 5;
            public const int IconPaletteHeight = 5;
            public const int IconsCount = 25;
        }
                
        protected static Bitmap demoIcons;

        public static ManagedBitmapServer Instance
        {
            get
            {
                if (m_instance == null)
                {
                    m_instance = new ManagedBitmapServer();
                }

                return m_instance;
            }
        }

        public static void StartServer()
        {
            BitmapServerProvider.setBitmapServerProvider(new BitmapServerProvider_SPtr(Instance));
        }

        public static void StopServer()
        {
            if (m_instance != null)
            {
                BitmapServerProvider.setBitmapServerProvider(new BitmapServerProvider_SPtr());
                m_instance = null;
            }
        }

        protected static ManagedBitmapServer m_instance = null;

        #endregion

        #region Members

        private BitmapServerCache Cache = new BitmapServerCache();

        #endregion

        #region IBitmapServerProvider interface

        /// <summary>
        /// Loads an Icon from a string the describe the Icon
        /// 
        /// the resulted string is build as follows:(int32 width)(int32 height)(bitmap data as 32bit_BRGA). 
        /// 
        /// if method failed. return a an empty string.
        /// </summary>
        /// <param name="iconStyle">iconStyle in XML format</param>        
        /// <returns>a string (in base64 encoding) with the bitmap data or empty string if icon loading failed</returns>       
        public override string loadIcon(string iconStyle)
        {
            //Create a bitmap image
            Bitmap bitmap = CreateIconBitmap(iconStyle);

            //if failes, display missing icon image...
            if (bitmap == null)
            {
                bitmap = LoadBitmapFromApplicationResources("missing_icon.png");
            }

            if (bitmap == null)
            {
                return "";
            }

            return System.Convert.ToBase64String(CreateByteArrayFromBitmap(bitmap));
        }

        public override string loadResource(string resourceName)
        {
            //Create a bitmap image
            Bitmap bitmap = LoadBitmapFromApplicationResources(resourceName);

            if (bitmap == null)
            {
                return "";
            }

            return System.Convert.ToBase64String(CreateByteArrayFromBitmap(bitmap));
        }

        public override string loadBitmap(string path)
        {
            //Create a bitmap image
            Bitmap bitmap = null;

            try
            {
                //try to create a bitmap from specific path
                bitmap = new Bitmap(path);

                //make sure it's ARGB data
                if (bitmap.PixelFormat != PixelFormat.Format32bppArgb)
                {
                    bitmap = new Bitmap(bitmap);
                }
            }
            catch (Exception)
            {
            }

            if (bitmap == null)
            {
                return "";
            }

            return System.Convert.ToBase64String(CreateByteArrayFromBitmap(bitmap));
        }

        public override string forceRGB(string path)
        {
            Bitmap rgbBitmap = null;

            //read the file and dispose of the file stream(avoid locking)
            using (var stream = new FileStream(path,FileMode.Open))
            {
                try
                {
                    //an exception will occur if the file data doesn't shape into a bitmap
                    var bitmap = new Bitmap(stream);
                    if (bitmap.PixelFormat != PixelFormat.Format32bppRgb)
                    {
                        rgbBitmap = bitmap.Clone(new Rectangle(Point.Empty, bitmap.Size), PixelFormat.Format32bppRgb);
                    }
                }
                catch (ArgumentException e)
                {
                    //log the failure to simplify investigating when this problem occurs 
                    Trace.error(string.Format("An exception occurred when trying to load a bitmap from a file stream: ", e.Message, " ;file path: ", path));
                    throw e;
                }
            }

            //if rgbBitmpa is not null - overwrite the file with RGB format.
            if (rgbBitmap != null)
            {
                if (Path.GetExtension(path).ToLower() == ".gif")
                {
                    //saving gif will automaticly reduce it to 8bpp if there are less then 256 colors.
                    //therefore, we have to use Tiff codec that accepts Encoder.ColorDepth property
                    var tifCodec = System.Drawing.Imaging.ImageCodecInfo.GetImageEncoders().FirstOrDefault(codec => codec.FormatID == System.Drawing.Imaging.ImageFormat.Tiff.Guid);
                    var paramters = new System.Drawing.Imaging.EncoderParameters(1);
                    paramters.Param[0] = new System.Drawing.Imaging.EncoderParameter(System.Drawing.Imaging.Encoder.ColorDepth, 24L);

                    path = Path.ChangeExtension(path, ".tif");
                    rgbBitmap.Save(path, tifCodec, paramters);
                }
                else
                {
                    rgbBitmap.Save(path);
                }

                rgbBitmap.Dispose();
            }

            return path;
        }

        #endregion

        #region BitmapServerProvider for C#

        public string BitmapToDataUrl(Image image)
        {
            using (var stream = new MemoryStream())
            {
                image.Save(stream,ImageFormat.Png);
                return string.Format("data:image/png;base64,{0}", Convert.ToBase64String(stream.GetBuffer()));
            }
        }

        public string BitmapToDataUrl(Icon icon)
        {
            using (var stream = new MemoryStream())
            {
                icon.Save(stream);
                return string.Format("data:image/x-icon;base64,{0}", Convert.ToBase64String(stream.GetBuffer()));
            }
        }

        public Bitmap LoadBitmapFromDefinition(string definition)
        {
            //Create a bitmap image
            Bitmap bitmap = CreateIconBitmap(definition);

            //if failes, display missing icon image...
            if (bitmap == null)
            {
                bitmap = LoadBitmapFromApplicationResources("missing_icon.png");
            }

            return bitmap;
        }

        public Bitmap LoadBitmapFromPath(string path)
        {
            //Create a bitmap image
            Bitmap bitmap = null;

            try
            {
                if (File.Exists(path))
                {
                    //load local file
                    bitmap = new Bitmap(path);
                }
                else if (System.Uri.IsWellFormedUriString(path, UriKind.Absolute))
                {
                    if (path.StartsWith("data:image/") && path.Contains(";base64,"))
                    {
                        //loading from Data scheme Uri only supported in .Net 4.5
                        var data = System.Convert.FromBase64String(path.Substring(path.IndexOf(";base64,") + ";base64,".Length));

                        using (var stream = new MemoryStream(data))
                        {
                            bitmap = new Bitmap(stream);
                        }
                    }
                    else
                    {
                        //do a web request
                        Uri uri = new Uri(path);

                        WebRequest request = WebRequest.Create(uri);

                        WebResponse response = request.GetResponse();

                        bitmap = new Bitmap(response.GetResponseStream());
                    }
                }
                else
                {
                    return null;
                }

                //make sure it's ARGB data
                if (bitmap.PixelFormat != PixelFormat.Format32bppArgb)
                {
                    bitmap = new Bitmap(bitmap);
                }
            }
            catch (Exception)
            {
            }

            return bitmap;
        }

        #endregion

        #region Parse Icon Style functions


        /// <summary>
        /// Create a bitmap from an icon style.
        /// </summary>
        /// <param name="iconStyle">iconStyle in XML format</param>        
        /// <returns>a Bitmap object or null if failed to load icon from the style</returns>       
        protected Bitmap CreateIconBitmap(string iconStyle)
        {
            Bitmap bitmap = Cache.Get(iconStyle);

            if (bitmap == null)
            {
                //if failed - bitmap would be null
                bitmap = TryLoadDemoIcon(iconStyle);

                if (bitmap == null)
                    bitmap = TryLoadXMLIconStyle(iconStyle);
            }

            if (bitmap != null)
            {
                Cache.Add(iconStyle, bitmap);
            }

            return bitmap;
        }

        private Bitmap TryLoadXMLIconStyle(string style)
        {
            Bitmap bitmap = null;

            try
            {
                XmlDocument iconStyle = new XmlDocument();
                iconStyle.LoadXml(style);

                foreach (XmlNode node in iconStyle.ChildNodes)
                {
                    bitmap = CreateBitmapFromXMLNode(node, bitmap);
                }
            }
            catch (Exception e)
            {
                Trace.error(String.Format("Failed to load icon from style: {0}\nException:{1}", style, e.ToString()));
            }            

            return bitmap;
        }

        private Bitmap CreateBitmapFromXMLNode(XmlNode node, Bitmap inputBitmap)
        {
            Bitmap cacheBitmap = Cache.Get(node.OuterXml);

            if (cacheBitmap != null)
            {
                return cacheBitmap;
            }

            switch (node.Name.ToLower())
            {
                case "text":
                    {
                        Font font = null;

                        FontStyle style = FontStyle.Regular;

                        if (node.Attributes["Style"] != null)
                        {
                            string[] styles = node.Attributes["Style"].Value.Split(new string[] { ", " }, StringSplitOptions.RemoveEmptyEntries);

                            foreach (string oneStyle in styles)
                            {
                                style |= (FontStyle)Enum.Parse(typeof(FontStyle), oneStyle);
                            }
                        }

                        if (node.Attributes["Font"] != null)
                        {
                            font = new Font(node.Attributes["Font"].Value, float.Parse(node.Attributes["Size"].Value), style, GraphicsUnit.Point);
                        }
                        else
                        {
                            font = new Font("Arial", 15.0f, style, GraphicsUnit.Point);
                        }

                        Brush brush = Brushes.White;

                        if (node.Attributes["Color"] != null)
                        {
                            brush = new SolidBrush(ParseColor(node.Attributes["Color"].Value));
                        }

                        return RenderText(node.InnerText, font, style, brush);
                    }

                case "src":
                    {
                        Bitmap bitmap = LoadBitmapFromPath(node.InnerText);

                        if (bitmap != null)
                        {
                            Cache.Add(node.OuterXml, bitmap);
                        }

                        return bitmap;
                    }

                case "crop":
                    {
                        Bitmap bitmap = CreateBitmapFromXMLNode(node.FirstChild, null);

                        Rectangle cropRegion = new Rectangle(new Point(0, 0), bitmap.Size);
                        if (node.Attributes["X"] != null)
                        {
                            cropRegion.X = int.Parse(node.Attributes["X"].Value);
                        }
                        if (node.Attributes["Y"] != null)
                        {
                            cropRegion.Y = int.Parse(node.Attributes["Y"].Value);
                        }
                        if (node.Attributes["Width"] != null)
                        {
                            cropRegion.Width = int.Parse(node.Attributes["Width"].Value);
                        }
                        if (node.Attributes["Height"] != null)
                        {
                            cropRegion.Height = int.Parse(node.Attributes["Height"].Value);
                        }

                        return Copy(bitmap, cropRegion);
                    }

                case "grid":
                    {
                        Bitmap bitmap = CreateBitmapFromXMLNode(node.FirstChild, null);

                        int rows = 1;
                        int cols = 1;
                        int index = 0;

                        if (node.Attributes["Rows"] != null)
                        {
                            rows = int.Parse(node.Attributes["Rows"].Value);
                        }
                        if (node.Attributes["Cols"] != null)
                        {
                            cols = int.Parse(node.Attributes["Cols"].Value);
                        }
                        if (node.Attributes["Index"] != null)
                        {
                            index = int.Parse(node.Attributes["Index"].Value);
                        }

                        return Copy(bitmap, CreateGridRegion(bitmap.Size, rows, cols, index));
                    }

                case "demo":
                    return TryLoadDemoIcon(node.Attributes["index"].Value);

                default:
                    return null;
            }
        }

        private Color ParseColor(string color)
        {
            try
            {
                return Color.FromName(color);
            }
            catch (Exception)
            {
            }

            try
            {
                return Color.FromArgb(int.Parse(color, System.Globalization.NumberStyles.HexNumber));
            }
            catch (Exception)
            {
            }

            throw new ArgumentException("Failed to parse color: '" + color + "'", "color");
        }
        
        #endregion

        #region bitmap manipulation functions
        /// <summary>
        /// Convert a bitmap object to a byte array
        /// </summary>
        /// <param name="bitmap">Bitmap object</param>        
        /// <returns>a string with the bitmap data in the following format: (int32 width)(int32 height)(bitmap data as 32bit_BRGA)</returns>       
        protected byte[] CreateByteArrayFromBitmap(Bitmap bitmap)
        {
            MemoryStream b = new MemoryStream();

            //write the width - 32 bit
            byte[] width = BitConverter.GetBytes(bitmap.Width);
            b.Write(width, 0, width.Length);

            //write the height - 32 bit
            byte[] height = BitConverter.GetBytes(bitmap.Height);
            b.Write(height, 0, height.Length);


            //lock bitmap data
            BitmapData bitmapData = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);

            //write the bitmap data - we have to copy the data before we write it to a MemoryStrem
            byte[] byteData = new byte[bitmapData.Height * bitmapData.Width * 4];
            System.Runtime.InteropServices.Marshal.Copy(bitmapData.Scan0, byteData, 0, byteData.Length);
            b.Write(byteData, 0, byteData.Length);

            //unlock bitmap data
            bitmap.UnlockBits(bitmapData);

            b.Position = 0;

            return b.ToArray();
        }

        /// <summary>
        /// Convert a bitmap object to a string
        /// </summary>
        /// <param name="bitmap">Bitmap object</param>        
        /// <returns>a string with the bitmap data in the following format: (int32 width)(int32 height)(bitmap data as 32bit_BRGA)</returns>       
        protected string CreateStringFromBitmap(Bitmap bitmap)
        {
            MemoryStream b = new MemoryStream();

            //write the width - 32 bit
            byte[] width = BitConverter.GetBytes(bitmap.Width);
            b.Write(width, 0, width.Length);

            //write the height - 32 bit
            byte[] height = BitConverter.GetBytes(bitmap.Height);
            b.Write(height, 0, height.Length);


            //lock bitmap data
            BitmapData bitmapData = bitmap.LockBits(new Rectangle(0, 0, bitmap.Width, bitmap.Height), ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);

            //write the bitmap data - we have to copy the data before we write it to a MemoryStrem
            byte[] byteData = new byte[bitmapData.Height * bitmapData.Width * 4];
            System.Runtime.InteropServices.Marshal.Copy(bitmapData.Scan0, byteData, 0, byteData.Length);
            b.Write(byteData, 0, byteData.Length);

            //unlock bitmap data
            bitmap.UnlockBits(bitmapData);

            //write a string
            StreamReader reader = new StreamReader(b);
            return reader.ReadToEnd();
        }

        /// <summary>
        /// Copy a section of a Bitmap to a new Bitmap object
        /// </summary>
        /// <param name="srcBitmap">Bitmap object</param>        
        /// <param name="section">Rectangle section to copy</param>        
        /// <returns>a new Bitmap object</returns>       
        protected Bitmap Copy(Bitmap srcBitmap, Rectangle section)
        {
            return srcBitmap.Clone(section, PixelFormat.Format32bppArgb);            
        }

        #endregion

        #region Demo Icon functions

        /// <summary>
        /// Loads an Icon from a the old style ApplyStyle format. which is a just a number that specifices the Index in the demo-icon file
        /// </summary>
        protected Bitmap TryLoadDemoIcon(string iconStyle)
        {
            int iconIndex = -1;
            if (!int.TryParse( iconStyle, out iconIndex))
            {
                //this is not a demo icon style
                return null;
            }

            LoadDemoIcons();

            //LoadDemoIcons failed...
            if (demoIcons == null)
            {
                return null;
            }

            //convert the iconIndex into bitmap section
            int row = iconIndex / DemoIconsInfo.IconPaletteWidth;
            int col = iconIndex % DemoIconsInfo.IconPaletteWidth;

            if (row >= DemoIconsInfo.IconPaletteHeight || col >= DemoIconsInfo.IconPaletteWidth || iconIndex >= DemoIconsInfo.IconsCount)
            {
                Trace.error("Failed to load demo-icon because iconIndex is out of range");
                return null;
            }

            Rectangle section = new Rectangle(col * DemoIconsInfo.IconWidth, row * DemoIconsInfo.IconHeight, DemoIconsInfo.IconWidth, DemoIconsInfo.IconHeight);

            //get the bitmap section
            Bitmap bitmap = Copy(demoIcons, section);

            return bitmap;
        }

        /// <summary>
        /// Loads demo-icons bitmap from application resource
        /// </summary>
        protected void LoadDemoIcons()
        {
            //great - the demoIcons were loaded alreay
            if (demoIcons != null)
            {
                return;
            }

            demoIcons = LoadBitmapFromApplicationResources(DemoIconsInfo.ResourceName);
        }

        Rectangle CreateGridRegion(Size bitmapSize, int rows, int cols, int iconIndex)
        {
            int row = iconIndex / cols;
            int col = iconIndex % cols;

            int IconWidth = bitmapSize.Width / cols;
            int IconHeight = bitmapSize.Height / rows;

            return new Rectangle(col * IconWidth, row * IconHeight, IconWidth, IconHeight);
        }

        #endregion

        #region Loading Application Resource

        List<System.Reflection.Assembly> m_searchableAssemblies = new List<System.Reflection.Assembly>();

        public void RegisterResourcesInAssembly(System.Reflection.Assembly assembly)
        {
            if (!m_searchableAssemblies.Contains(assembly))
            {
                m_searchableAssemblies.Insert(0, assembly);
            }
        }

        public Bitmap LoadBitmapFromApplicationResources(string resourceName)
        {
            Bitmap bitmap = null;

            foreach (var assembly in m_searchableAssemblies)
            {
                foreach (string name in assembly.GetManifestResourceNames())
                {
                    if (name.EndsWith(resourceName))
                    {
                        try
                        {
                            bitmap = new Bitmap(assembly.GetManifestResourceStream(name));

                            Trace.info("bitmap was successfully loaded");
                        }
                        catch (Exception e)
                        {
                            Trace.error("Failed to bitmap from resource: " + e.Message);
                        }

                        return bitmap;
                    }
                }
            }

            Trace.error("Failed to find '" + resourceName + "' resource name. Couldn't load bitmap");

            return null;
        }

        #endregion

        #region Font Rendering

        protected Bitmap RenderText(string text, Font font, FontStyle fontStyle, Brush brush)
        {
            //if there is no text - return small empty image
            if (String.IsNullOrEmpty(text))
            {
                return new Bitmap(1, 1);
            }

            Bitmap bitmap = new Bitmap(1, 1);
            Graphics g = Graphics.FromImage(bitmap);

            //Calculate the rect needed to render string. Note: the maximum width can't be more then 256 pixels (Max size for TexturePacker textures)
            RectangleF rect = new RectangleF(new PointF(0.0f, 0.0f), g.MeasureString(text, font, 256));

            //if rect has no size - return small empty image
            if ((int)rect.Height == 0 && (int)rect.Width == 0)
            {
                return new Bitmap(1,1);
            }

            //Create new bitmap in the right size
            bitmap = new Bitmap((int)rect.Width, (int)rect.Height);
            g = Graphics.FromImage(bitmap);

            //Create a path from wanted string
            System.Drawing.Drawing2D.GraphicsPath path = new System.Drawing.Drawing2D.GraphicsPath();

            StringFormat stringFormat = new StringFormat();
            stringFormat.Alignment = StringAlignment.Center;            

            path.AddString(text, font.FontFamily, (int)fontStyle, font.SizeInPoints, rect, stringFormat);
            
            //Make sure the Graphics do it's best jub for AntiAliasing
            g.SmoothingMode = System.Drawing.Drawing2D.SmoothingMode.HighQuality;

            //Create a big and semi-transparent Pen.
            Pen p = new Pen(Color.FromArgb(128, Color.Black), 6.0f);
            p.LineJoin = System.Drawing.Drawing2D.LineJoin.Round;

            //Draw the black outline for the text
            for (int i = 6; i > 0; i -= 2)
            {
                p.Width = i;
                g.DrawPath(p, path);
            }

            //Draw the text itself with the requested brush
            g.FillPath(brush, path);

            return bitmap;
        }

        #endregion

        #region PYXObject Lifetime Management

        #region IDirectorReferenceCounter Members

        public void setSwigCMemOwn(bool value)
        {
            swigCMemOwn = value;
        }

        public int doAddRef()
        {
            return base.addRef();
        }

        public int doRelease()
        {
            return base.release();
        }

        #endregion

        #region PYXObject

        /// <summary>
        /// Override the reference-counting addRef.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after increment).</returns>
        public override int addRef()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.addRef(this);
        }

        /// <summary>
        /// Override the reference-counting release.  This is not called 
        /// directly!
        /// </summary>
        /// <returns>Current reference count (after decrement).</returns>
        public override int release()
        {
            if (getCPtr(this).Handle == IntPtr.Zero)
            {
                return 1;
            }
            return ReferenceManager.release(this);            
        }

        #endregion

        /// <summary>
        /// Protected Default constructor - force it to create only one instance
        /// </summary>
        private ManagedBitmapServer()
        {
            m_searchableAssemblies.Add(System.Reflection.Assembly.GetEntryAssembly());
        }
        #endregion PYXObject Lifetime Management
    }


    internal class BitmapServerCache
    {
        protected Dictionary<string, Bitmap> cache = new Dictionary<string,Bitmap>();
        protected List<string> usage = new List<string>();        

        public int CacheSizeLimit = 10;

        public Bitmap Get(string xmlString)
        {
            if (cache.ContainsKey(xmlString))
            {
                updateUsage(xmlString);

                return cache[xmlString];
            }
            return null;
        }

        public void Add(string xmlString, Bitmap bitmap)
        {
            if (!cache.ContainsKey(xmlString))
            {
                //force cache limit
                if (cache.Count >= CacheSizeLimit)
                {
                    cache.Remove(usage[0]);
                    usage.RemoveAt(0);
                }
            }

            //update cache entry
            cache[xmlString] = bitmap;
            updateUsage(xmlString);
        }

        protected void updateUsage(string xmlString)
        {
            //update usage
            if (usage.Contains(xmlString))
            {
                usage.Remove(xmlString);
            }
            usage.Add(xmlString);
        }
    }
}
