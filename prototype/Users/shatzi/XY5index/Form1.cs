using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Text;
using System.Windows.Forms;

namespace XY5index
{
    public partial class Form1 : Form
    {
        int resolution = 1;
        bool showText = true;
        double edgeLength = 200;
        
        PYX5Cursor cursor = new PYX5Cursor(new PYX5index(2,2,0,1),1);

        public Form1()
        {
            InitializeComponent();

            gridPictureBox.Width = (int)(edgeLength * 1.5 * 7);
            gridPictureBox.Height = (int)(edgeLength * 4);
        }

        private void gridPictureBox_Resize(object sender, EventArgs e)
        {
            gridPictureBox.Invalidate();
        }

        private void gridPictureBox_Paint(object sender, PaintEventArgs e)
        {            
            for (int z = 0; z < 5; z++)
            {
                DrawStrip(e,z,resolution);
            }
        }

        private void DrawStrip(PaintEventArgs e,int z,int resolution)
        {
            Point zero = new Point((int)(z * edgeLength * 1.5 + edgeLength * 1.5), 50);
            Point lastPoint = zero;
            Point point = zero;
            point.Offset((int)(2 * edgeLength * Math.Cos(Math.PI * 4 / 6)), (int)(2 * edgeLength * Math.Sin(Math.PI * 4 / 6)));
            e.Graphics.DrawLine(Pens.Black, lastPoint, point);

            lastPoint = point;
            point.Offset((int)(edgeLength * Math.Cos(Math.PI * 2 / 6)), (int)(edgeLength * Math.Sin(Math.PI * 2 / 6)));
            e.Graphics.DrawLine(Pens.Black, lastPoint, point);

            lastPoint = point;
            point.Offset((int)(2 * edgeLength * Math.Cos(Math.PI * 4 / 6 + Math.PI)), (int)(2 * edgeLength * Math.Sin(Math.PI * 4 / 6 + Math.PI)));
            e.Graphics.DrawLine(Pens.Black, lastPoint, point);

            lastPoint = point;
            point = zero;
            e.Graphics.DrawLine(Pens.Black, lastPoint, point);

            int r = resolution;

            if (resolution % 2 == 0)
            {
                r++;
            }

            for (int x = 0; x < PYX5index.GetMaxXValue(r); x++)
            {
                for (int y = 0; y < PYX5index.GetMaxYValue(r); y++)
                {
                    if (resolution % 2 != 0 || (x - y) % 3 == 0)
                    {                       
                        if (cursor.Index.IsSame(new PYX5index(x, y, z, resolution)))
                        {
                            DrawHexagon(e, zero, x, y, resolution, new Pen(Color.Green, 4));
                            continue;
                        }                                                
                        DrawHexagon(e, zero, x, y, resolution, new Pen(Color.Red, 2));                        
                    }
                }
            }         
        }

        private void DrawHexagon(PaintEventArgs e, Point zero, int x, int y, int resolution,Pen pen)
        {
            double radius = edgeLength / (2 * Math.Pow(Math.Sqrt(3), resolution + 1));
            double step = radius;

            double angleOffset = 0;
            if (resolution % 2 != 0)
            {
                angleOffset = Math.PI / 2;
            }
            else
            {
                step = edgeLength / (2 * Math.Pow(Math.Sqrt(3), resolution + 2));
            }

            zero.Offset((int)(2 * x * step * Math.Cos(Math.PI * 4 / 6)), (int)(2 * x * step * Math.Sin(Math.PI * 4 / 6)));
            zero.Offset((int)(2 * y * step * Math.Cos(Math.PI * 2 / 6)), (int)(2 * y * step * Math.Sin(Math.PI * 2 / 6)));

            
            for (int i =0; i<6;i++)
            {
                Point p1 = zero;
                p1.Offset((int)(radius / Math.Sin(Math.PI * 4 / 6) * Math.Cos(i * Math.PI * 2 / 6 + angleOffset)), (int)(radius / Math.Sin(Math.PI * 4 / 6) * Math.Sin(i * Math.PI * 2 / 6 + angleOffset)));

                Point p2 = zero;
                p2.Offset((int)(radius / Math.Sin(Math.PI * 4 / 6) * Math.Cos((i + 1) * Math.PI * 2 / 6 + angleOffset)), (int)(radius / Math.Sin(Math.PI * 4 / 6) * Math.Sin((i + 1) * Math.PI * 2 / 6 + angleOffset)));

                e.Graphics.DrawLine(pen, p1, p2);
            }

            string indexString = String.Format("({0},{1})", x, y);
            SizeF s = e.Graphics.MeasureString(indexString, this.Font);
            e.Graphics.DrawString(indexString, this.Font, Brushes.Black, new PointF(zero.X - s.Width / 2, zero.Y - s.Height / 2));
        }

        private void upToolStripButton_Click(object sender, EventArgs e)
        {
            if (resolution > 0)
            {
                resolution -= 1;
            }
            gridPictureBox.Invalidate();
        }

        private void downToolStripButton_Click(object sender, EventArgs e)
        {
            resolution += 1;
            gridPictureBox.Invalidate();
        }

        private void zoomInToolStripButton_Click(object sender, EventArgs e)
        {
            edgeLength *= 1.5;
            gridPictureBox.Width = (int)(edgeLength * 1.5 * 7);
            gridPictureBox.Height = (int)(edgeLength * 4);
            gridPictureBox.Invalidate();
        }

        private void zoomOutToolStripButton_Click(object sender, EventArgs e)
        {
            edgeLength /= 1.5;
            gridPictureBox.Width = (int)(edgeLength * 1.5 * 7);
            gridPictureBox.Height = (int)(edgeLength * 4);
            gridPictureBox.Invalidate();            
        }

        private void gridPictureBox_MouseUp(object sender, MouseEventArgs e)
        {
            for (int z = 0; z < 5; z++)
            {
                Point zero = new Point((int)(z * edgeLength * 1.5 + edgeLength * 1.5), 50);

                int m_x = e.X - zero.X;
                int m_y = e.Y - zero.Y;
                
                double x = - 0.5 * m_x / (edgeLength * Math.Sin(Math.PI / 6)) + 0.5 * m_y / (edgeLength * Math.Cos(Math.PI / 6));
                double y = 0.5 * m_x / (edgeLength * Math.Sin(Math.PI / 6)) + 0.5 * m_y / (edgeLength * Math.Cos(Math.PI / 6));
                
                if (x >= 0 && x <= 2 && y >= 0 && y <= 1)
                {
                    cursor.Index = PYX5index.FromStripLocation(x, y, z, resolution);
                    break;
                }
            }

            gridPictureBox.Invalidate();
        }

        private void Form1_KeyUp(object sender, KeyEventArgs e)
        {
           
        }

        private void toolStripContainer1_KeyUp(object sender, KeyEventArgs e)
        {
            switch (e.KeyCode)
            {
                case Keys.Up:
                    cursor.Forward(1);
                    break;

                case Keys.Left:
                    cursor.Left();
                    break;

                case Keys.Right:
                    cursor.Right();
                    break;
            }

            gridPictureBox.Invalidate();
        }             
    }

    public class PYX5index
    {
        public int X = 0;
        public int Y = 0;
        public int Z = 0;

        public int Resolution = 0;

        public PYX5index(int x, int y, int z, int resolution)
        {
            X = x;
            Y = y;
            Z = z;
            Resolution = resolution;
        }

        public static PYX5index FromStripLocation(double x, double y, int z, int resolution)
        {
            //TODO: solve even-resolutions
            int x_max = PYX5index.GetMaxXValue(resolution)-1;
            int y_max = PYX5index.GetMaxYValue(resolution)-1;
            x /= 2;

            x = Math.Round(x_max * x);
            y = Math.Round(y_max * y);
            return new PYX5index((int)(x), (int)(y), z, resolution);            
        }

        public override bool Equals(object obj)
        {
            if (obj == null)
                return false;

            if (obj is PYX5index)
            {                
                return IsSame(obj as PYX5index);
            }

            return base.Equals(obj);
        }

        public bool IsEqual(PYX5index b)
        {
            return X == b.X && Y == b.Y && Z == b.Z && Resolution == b.Resolution;
        }

        public bool IsSame(PYX5index b)
        {
            if (!isBorder())
            {
                return IsEqual(b);
            }
            else
            {
                if (IsEqual(b))
                {
                    return true;
                }

                if (Resolution != b.Resolution)
                {
                    return false;
                }

                if (!b.isBorder())
                {
                    return false;
                }

                if (b.isNorthPool() && isNorthPool())
                {
                    return true;
                }

                if (b.isSouthPool() && isSouthPool())
                {
                    return true;
                }

                return JumpBorder().IsEqual(b);
            }
        }

        static public int GetMaxXValue(int resolution)
        {
            return (int)(2*3 * Math.Pow(3, (resolution - 1)/2.0)+1);
        }

        static public int GetMaxYValue(int resolution)
        {
            return (int)(3 * Math.Pow(3, (resolution - 1)/2.0)+1);
        }

        public bool isValid()
        {
            return (X >= 0 && Y >= 0 && X < GetMaxXValue(Resolution) && Y < GetMaxYValue(Resolution));            
        }

        public bool isBorder()
        {
            return (X == 0 || Y == 0 || X == GetMaxXValue(Resolution) - 1 || Y == GetMaxYValue(Resolution) - 1);            
        }

        public bool isPositiveBorder()
        {
            return (X == 0 || Y == GetMaxYValue(Resolution) - 1);            
        }

        public bool isNegativeBorder()
        {
            return (Y == 0 || X == GetMaxXValue(Resolution) - 1 );
        }

        public bool isPool()
        {
            return isNorthPool() || isNorthPool();
        }

        public bool isNorthPool()
        {
            return X == 0 && Y == 0;
        }

        public bool isSouthPool()
        {
            return X == GetMaxXValue(Resolution)-1 && Y == GetMaxYValue(Resolution)-1;
        }

        public int GetLine()
        {
            return X+Y;
        }

        public int GetSector()
        {
            int line = GetLine();
            int edgeLength = GetMaxYValue(Resolution) - 1;
            if (line < edgeLength)
                return 0;
            if (line > edgeLength*2)
                return 2;

            return 1;
        }

        public PYX5index JumpBorder()
        {
            if (!isBorder() || isPool())
            {
                return this;
            }

            int line = GetLine();
            int edgeLines = GetMaxYValue(Resolution) - 1;
            int step = edgeLines;

            if (line < edgeLines)
            {
                step = line;
            }
            else if (line > edgeLines * 2)
            {
                step = edgeLines * 3 - line;
            }

            if (X == 0 || Y == GetMaxYValue(Resolution)-1)
            {
                return new PYX5index(X+step, Y-step, (Z+1) % 5, Resolution);
            }

            if (Y == 0 || X == GetMaxXValue(Resolution) - 1)
            {
                return new PYX5index(X - step, Y + step, (Z+4) % 5, Resolution);
            }

            throw new Exception("Can't reach here");
        }

        static public PYX5index operator +(PYX5index index, PYX5IndexOffset offset)
        {
            return new PYX5index(index.X + offset.X, index.Y + offset.Y, index.Z + offset.Z,index.Resolution);
        }

        static public PYX5index operator -(PYX5index index, PYX5IndexOffset offset)
        {
            return new PYX5index(index.X - offset.X, index.Y - offset.Y, index.Z - offset.Z, index.Resolution);
        }
    }

    public class PYX5IndexOffset
    {
        public int X;
        public int Y;
        public int Z;

        public PYX5IndexOffset(int x, int y, int z)
        {
            X = x;
            Y = y;
            Z = z;
        }

        public static PYX5IndexOffset FromDirection(int direction)
        {
            switch (direction%6)
            {
                case 0:
                    return new PYX5IndexOffset(-1, 0, 0);

                case 1:
                    return new PYX5IndexOffset(0, -1, 0);

                case 2:
                    return new PYX5IndexOffset(1, -1, 0);

                case 3:
                    return new PYX5IndexOffset(1, 0, 0);

                case 4:
                    return new PYX5IndexOffset(0, 1, 0);

                case 5:
                    return new PYX5IndexOffset(-1, 1, 0);

                default:
                    throw new Exception("Now good");
            }
        }
    }
    
    public class PYX5Cursor
    {
        public PYX5index Index;
        public int Direction;

        public PYX5Cursor(PYX5index index, int direction)
        {
            Index = index;
            Direction = direction;
        }

        public void ZoomIn(int amount)
        {
            int jump = (Index.Resolution%2+amount)/2;
            Index.X *= (int)(Math.Pow(3, jump));
            Index.Y *= (int)(Math.Pow(3, jump));
            Index.Resolution += amount;
        }

        public void ZoomOut(int amount)
        {
            if (Index.Resolution < amount)
            {
                throw new Exception("Not good");
            }

            int jump = (Index.Resolution % 2 + amount) / 2;
            Index.X /= (int)(Math.Pow(3, jump));
            Index.Y /= (int)(Math.Pow(3, jump));
            Index.Resolution -= amount;
        }

        public void Left()
        {
            Direction = (Direction + 1) % 6;
        }

        public void Right()
        {
            Direction = (Direction + 5) % 6;
        }

        public void Forward(int amount)
        {
            for (int i = 0; i < amount; i++)
            {
                PYX5index newIndex = Index + PYX5IndexOffset.FromDirection(Direction);

                if (newIndex.isValid())
                {
                    Index = newIndex;
                }
                else
                {
                    
                    switch (Index.GetSector())
                    {
                        case 0:
                            if (Index.isPositiveBorder())
                            {
                                Right();
                            }

                            if (Index.isNegativeBorder())
                            {
                                Left();
                            }
                            break;
                        case 2:
                            if (Index.isPositiveBorder())
                            {
                                Left();
                            }

                            if (Index.isNegativeBorder())
                            {
                                Right();
                            }
                            break;
                        default:
                            break;
                    }
                    

                    Index = Index.JumpBorder() + PYX5IndexOffset.FromDirection(Direction);                    
                }
            }
        }

        public void Backward(int amount)
        {
            throw new NotImplementedException();
        }
    }
}
