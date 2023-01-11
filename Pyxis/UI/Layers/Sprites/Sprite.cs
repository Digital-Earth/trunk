using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Windows.Forms;
using Tao.OpenGl;

namespace Pyxis.UI.Layers.Sprites
{
    /// <summary>
    /// A graphics sprite to display on a Pyxis.UI.ILayer.
    /// </summary>
    public class Sprite 
    {
        /// <summary>
        /// Gets or sets the location.
        /// </summary>
        public PointF Location { get; set; }
        /// <summary>
        /// Gets or sets the texture.
        /// </summary>
        public Texture Texture { get; set; }
        /// <summary>
        /// Gets or sets the rotation angle in degrees.
        /// </summary>
        public float RotationAngle { get; set; }
        /// <summary>
        /// Gets or sets the size.
        /// </summary>
        public SizeF Size { get; set; }
        /// <summary>
        /// Gets or sets the scale.
        /// </summary>
        public SizeF Scale { get; set; }
        /// <summary>
        /// Gets or sets the color.
        /// </summary>
        public Color Color { get; set; }
        /// <summary>
        /// Gets or sets the opacity.
        /// </summary>
        public double Opacity { get; set; }
        /// <summary>
        /// Gets or sets the alignment anchor.
        /// </summary>
        public ContentAlignment Anchor { get; set; }

        /// <summary>
        /// Gets the parent of the Pyxis.UI.Layers.Sprites.Sprite.
        /// </summary>
        public Sprite Parent { get; private set; }
        /// <summary>
        /// Gets the children of the Pyxis.UI.Layers.Sprites.Sprite.
        /// </summary>
        public List<Sprite> Children { get; private set; }
        /// <summary>
        /// Gets the graphics library context.
        /// </summary>
        public IGlContext Context { get; protected set;}
        /// <summary>
        /// Gets or sets if the Pyxis.UI.Layers.Sprites.Sprite is visible.
        /// </summary>
        public bool Visible { get; set; }

        private static double s_zeroRotationThreshold = 0.0001;

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Sprites.Sprite class.
        /// </summary>
        /// <param name="parent">The parent sprite of the initialized Pyxis.UI.Layers.Sprites.Sprite class.</param>
        public Sprite(Sprite parent)
        {
            Visible = true;
            Opacity = 1.0;
            Color = Color.White;
            Scale = new SizeF(1, 1);
            Location = PointF.Empty;
            Anchor = (parent is RootSprite) ? ContentAlignment.TopLeft : ContentAlignment.MiddleCenter;
            Children = new List<Sprite>();

            if (parent != null)
            {
                Parent = parent;
                Context = parent.Context;
                parent.Children.Add(this);
            }
        }

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Sprites.Sprite class of a specified size.
        /// </summary>
        /// <param name="parent">The parent sprite of the initialized Pyxis.UI.Layers.Sprites.Sprite.</param>
        /// <param name="size">Size of the initialized Pyxis.UI.Layers.Sprites.Sprite.</param>
        public Sprite(Sprite parent, SizeF size)
            : this(parent)
        {
            Size = size;
        }

        /// <summary>
        /// Initializes a new instance of the Pyxis.UI.Layers.Sprites.Sprite class with a Pyxis.UI.Layers.Sprites.Texture.
        /// </summary>
        /// <param name="parent">The parent sprite of the initialized Pyxis.UI.Layers.Sprites.Sprite.</param>
        /// <param name="texture">The texture of the initialized Pyxis.UI.Layers.Sprites.Sprite.</param>
        public Sprite(Sprite parent, Texture texture)
            : this(parent)
        {
            Texture = texture;
            Size = texture != null ? new SizeF(texture.Width, texture.Height) : new SizeF(16, 16);
        }

        /// <summary>
        /// Draw the Pyxis.UI.Layers.Sprites.Sprite and all of its children.
        /// </summary>
        public virtual void Draw()
        {
            if (!Visible)
            {
                return;
            }

            Gl.glPushMatrix();

            var anchor = Parent.GetAnchor(Anchor);
            var halfWidth = Size.Width / 2;
            var halfHeight = Size.Height / 2;

            Gl.glTranslatef(anchor.X + Location.X, anchor.Y - Location.Y, 0);
            if (Math.Abs(RotationAngle) > s_zeroRotationThreshold)
            {
                Gl.glRotated(RotationAngle, 0, 0, 1);
            }
            Gl.glScalef(Scale.Width, Scale.Height, 1.0f);

            if (Texture != null)
            {
                Gl.glEnable(Gl.GL_TEXTURE_2D);
                Texture.Bind();
            }
            else
            {
                Gl.glDisable(Gl.GL_TEXTURE_2D);
            }

            Gl.glBegin(Gl.GL_QUADS);

            Gl.glColor4f(Color.R / 255.0f, Color.G / 255.0f, Color.B / 255.0f, (float)(Color.A / 255.0f * Opacity));

            Gl.glTexCoord2f(0, 1); Gl.glVertex3d(-halfWidth, -halfHeight, 0);
            Gl.glTexCoord2f(1, 1); Gl.glVertex3d(halfWidth, -halfHeight, 0);
            Gl.glTexCoord2f(1, 0); Gl.glVertex3d(halfWidth, +halfHeight, 0);
            Gl.glTexCoord2f(0, 0); Gl.glVertex3d(-halfWidth, +halfHeight, 0);

            Gl.glEnd();

            foreach (var child in Children)
            {
                child.Draw();
            }

            if (Texture != null)
            {
                Texture.Unbind();
            }

            Gl.glPopMatrix();

        }

        /// <summary>
        /// Get the content anchor point relative to the Pyxis.UI.Layers.Sprites.Sprite given a System.Drawing.ContentAlignment.
        /// </summary>
        /// <param name="anchor">The alignment of the anchor point.</param>
        /// <returns>System.Drawing.PointF describing the desired anchor point of the Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public PointF GetAnchor(ContentAlignment anchor)
        {
            switch (anchor)
            {
                default:
                case ContentAlignment.TopLeft:
                    return new PointF(-Size.Width / 2, Size.Height / 2);
                case ContentAlignment.TopCenter:
                    return new PointF(0, Size.Height / 2);
                case ContentAlignment.TopRight:
                    return new PointF(Size.Width / 2, Size.Height / 2);

                case ContentAlignment.MiddleLeft:
                    return new PointF(-Size.Width / 2, 0);
                case ContentAlignment.MiddleCenter:
                    return new PointF(0, 0);
                case ContentAlignment.MiddleRight:
                    return new PointF(Size.Width / 2, 0);

                case ContentAlignment.BottomLeft:
                    return new PointF(-Size.Width / 2, -Size.Height / 2);
                case ContentAlignment.BottomCenter:
                    return new PointF(0, -Size.Height / 2);
                case ContentAlignment.BottomRight:
                    return new PointF(Size.Width / 2, -Size.Height / 2);
            }
        }

        /// <summary>
        /// Get the local coordinate (relative to the Pyxis.UI.Layers.Sprites.Sprite) of a System.Drawing.PointF.
        /// </summary>
        /// <param name="point">System.Drawing.PointF to find in the local coordinate system.</param>
        /// <returns>The System.Drawing.PointF representing <paramref name="point"/> in the local coordinate system.</returns>
        public PointF ToLocalCoordinates(PointF point)
        {
            var anchor = Parent != null ? Parent.GetAnchor(Anchor) : PointF.Empty;
            //-translate;
            var local = new PointF(point.X - anchor.X - Location.X, point.Y + anchor.Y - Location.Y);

            //rotate in -rotationAngle
            if (Math.Abs(RotationAngle) > s_zeroRotationThreshold)
            {
                var c = (float)Math.Cos(-RotationAngle * Math.PI / 180.0);
                var s = (float)Math.Sin(-RotationAngle * Math.PI / 180.0);
                local = new PointF(c * local.X - s * local.Y, s * local.X + c * local.Y);
            }

            //unscale
            local = new PointF(local.X / Scale.Width, local.Y / Scale.Height);
            return local;
        }

        /// <summary>
        /// Determine if a System.Drawing.PointF in the local coordinate system is within the area of the Pyxis.UI.Layers.Sprites.Sprite.
        /// </summary>
        /// <param name="point">System.Drawing.PointF to test.</param>
        /// <returns>true if <paramref name="point"/> is within the area of the Pyxis.UI.Layers.Sprites.Sprite; otherwise, false.</returns>
        public bool HitTestWithLocalCoordinate(PointF point)
        {
            return Math.Abs(point.X) < Size.Width / 2 && Math.Abs(point.Y) < Size.Height / 2;
        }

        /// <summary>
        /// Occurs when the mouse pointer moves while over this Pyxis.UI.Layers.Sprites.Sprite.
        /// </summary>
        public EventHandler<MouseEventArgs> MouseMove { get; set; }
        /// <summary>
        /// Occurs when any mouse button is released over this Pyxis.UI.Layers.Sprites.Sprite.
        /// </summary>
        public EventHandler<MouseEventArgs> MouseUp { get; set; }
        /// <summary>
        /// Occurs when any mouse button is pressed while the pointer is over this Pyxis.UI.Layers.Sprites.Sprite.
        /// </summary>
        public EventHandler<MouseEventArgs> MouseDown { get; set; }
        /// <summary>
        /// Occurs when any mouse button is clicked while the pointer is over this Pyxis.UI.Layers.Sprites.Sprite.
        /// </summary>
        public EventHandler<MouseEventArgs> MouseClick { get; set; }
        /// <summary>
        /// Occurs when a mouse button is clicked two or more times while the pointer is over this Pyxis.UI.Layers.Sprites.Sprite.
        /// </summary>
        public EventHandler<MouseEventArgs> MouseDoubleClick { get; set; }

        /// <summary>
        ///  Represents the method that will handle the MouseMove event of the Pyxis.UI.Layers.Sprites.Sprite.
        ///  Recursively attempts to have children sprites handle the mouse event in depth-first order.
        /// </summary>
        /// <param name="args">A MouseEventArgs that contains the event data.</param>
        /// <param name="coordinate">The mouse coordinate when the event occurred.</param>
        /// <param name="getHandler">System.Func to handle the mouse event.</param>
        /// <returns>true if the event was handled; otherwise, false.</returns>
        internal bool HandleMouseEvent(MouseEventArgs args, PointF coordinate, Func<Sprite, EventHandler<MouseEventArgs>> getHandler)
        {
            if (!Visible)
            {
                return false;
            }

            var local = ToLocalCoordinates(coordinate);

            foreach (var child in Children.AsEnumerable().Reverse())
            {
                if (child.HandleMouseEvent(args, local, getHandler))
                {
                    return true;
                }
            }

            if (HitTestWithLocalCoordinate(local))
            {
                var handler = getHandler(this);
                if (handler != null)
                {
                    handler.Invoke(this, args);
                    return true;
                }
            }

            return false;
        }

        /// <summary>
        /// Get all the Pyxis.UI.Layers.Sprites.Sprite at a specific screen location.
        /// </summary>
        /// <param name="coordinate">A System.Drawing.PointF representing the location of interest on the screen.</param>
        /// <returns>The Pyxis.UI.Layers.Sprites.Sprite at <paramref name="coordinate"/>.</returns>
        public IEnumerable<Sprite> GetSpritesFromScreenLocation(PointF coordinate)
        {
            if (!Visible)
            {
                yield break;
            }

            var local = ToLocalCoordinates(coordinate);

            if (HitTestWithLocalCoordinate(local))
            {
                foreach (var child in Children.AsEnumerable().Reverse())
                {
                    foreach (var sprite in child.GetSpritesFromScreenLocation(local))
                    {
                        yield return sprite;
                    }
                }
                yield return this;
            }
        }

        /// <summary>
        /// Add a child Pyxis.UI.Layers.Sprites.Sprite initialized with a System.Drawing.Bitmap for texture.
        /// </summary>
        /// <param name="bitmap">System.Drawing.Bitmap to initialize the child's texture.</param>
        /// <returns>The initialized and added child Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite AddChildSprite(Bitmap bitmap)
        {
            var texture = new Texture(Context, bitmap);
            var sprite = new Sprite(this, texture);
            return sprite;
        }
        
        /// <summary>
        /// Add a child Pyxis.UI.Layers.Sprites.Sprite initialized with a Pyxis.UI.Layers.Sprites.Texture.
        /// </summary>
        /// <param name="texture">Pyxis.UI.Layers.Sprites.Texture to initialize the child's texture.</param>
        /// <returns>The initialized and added child Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite AddChildSprite(Texture texture)
        {
            var sprite = new Sprite(this, texture);
            return sprite;
        }

        /// <summary>
        /// Add a child Pyxis.UI.Layers.Sprites.Sprite initialized with a System.Drawing.SizeF.
        /// </summary>
        /// <param name="size">The size of the child.</param>
        /// <returns>The initialized and added child Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite AddChildSprite(SizeF size)
        {
            var sprite = new Sprite(this, size);
            return sprite;
        }

        /// <summary>
        /// Sets the Pyxis.UI.Layers.Sprites.Sprite location with a System.Drawing.PointF.
        /// </summary>
        /// <param name="point">System.Drawing.PointF to set the location with.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetLocation(PointF point)
        {
            Location = point;
            return this;
        }
        
        /// <summary>
        /// Sets the Pyxis.UI.Layers.Sprites.Sprite location with a System.Drawing.Point.
        /// </summary>
        /// <param name="point">System.Drawing.Point to set the location with.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetLocation(Point point)
        {
            Location = new PointF(point.X,point.Y);
            return this;
        }

        /// <summary>
        /// Sets the Pyxis.UI.Layers.Sprites.Sprite location with two floating-point numbers representing the location coordinates.
        /// </summary>
        /// <param name="x">The x-coordinate of the desired location.</param>
        /// <param name="y">The y-coordinate of the desired location.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetLocation(float x, float y)
        {
            Location = new PointF(x, y);
            return this;
        }

        /// <summary>
        /// Sets the opacity.
        /// </summary>
        /// <param name="opacity">Desired opacity.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetOpacity(double opacity)
        {
            Opacity = opacity;
            return this;
        }

        /// <summary>
        /// Sets the anchor.
        /// </summary>
        /// <param name="anchor">Desired anchor.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetAnchor(ContentAlignment anchor)
        {
            Anchor = anchor;
            return this;
        }

        /// <summary>
        /// Sets the rotation angle.
        /// </summary>
        /// <param name="angle">Desired rotation angle.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetRotation(float angle)
        {
            RotationAngle = angle;
            return this;
        }

        /// <summary>
        /// Sets the size to the specified width and height.
        /// </summary>
        /// <param name="width">Desired width.</param>
        /// <param name="height">Desired height.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetSize(float width,float height)
        {
            Size = new SizeF(width,height);
            return this;
        }

        /// <summary>
        /// Sets the size using a System.Drawing.SizeF.
        /// </summary>
        /// <param name="size">Desired size.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetSize(SizeF size)
        {
            Size = size;
            return this;
        }

        /// <summary>
        /// Sets the scale using a scaling factor.
        /// </summary>
        /// <param name="factor">Desired scaling factor.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetScale(float factor)
        {
            Scale = new SizeF(factor, factor);
            return this;
        }

        /// <summary>
        /// Sets the scale using separate width and height scaling factors.
        /// </summary>
        /// <param name="widthFactor">Desired width scaling factor.</param>
        /// <param name="heightFactor">Desired height scaling factor.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetScale(float widthFactor, float heightFactor)
        {
            Scale = new SizeF(widthFactor, heightFactor);
            return this;
        }

        /// <summary>
        /// Sets the color.
        /// </summary>
        /// <param name="color">Desired color.</param>
        /// <returns>The current Pyxis.UI.Layers.Sprites.Sprite.</returns>
        public Sprite SetColor(Color color)
        {
            Color = color;
            return this;
        }
    }
}
