using System;
using System.Drawing;
using System.Drawing.Drawing2D;

namespace nNet.mjpeg
{
    public class BitmapUtils
    {
        public static void ResizeToBitmap(Bitmap source, Bitmap target)
        {
            using (var g = Graphics.FromImage(target))
            {
                g.InterpolationMode = InterpolationMode.Low;
                g.DrawImage(source, 0, 0, target.Width, target.Height);
            }
        }

        public static void BitmapToNormalizedFloats(byte[] input, Bitmap b)
        {
            var w = b.Width;
            var h = b.Height;
            var spacial = w * h;
            Color px;
            
            for (var y = 0; y < h; y++)
            {
                for (var x = 0; x < w; x++)
                {
                    px = b.GetPixel(x, y);

                    int posIx = x + y * w;
                    int rIx = posIx;
                    int bIx = posIx + spacial;
                    int gIx = posIx + spacial * 2;

                    var rBits = ByteToNormFloat(px.R);
                    rBits.CopyTo(input, rIx * sizeof(float));

                    var gBits = ByteToNormFloat(px.G);
                    gBits.CopyTo(input, gIx * sizeof(float));

                    var bBits = ByteToNormFloat(px.B);
                    bBits.CopyTo(input, bIx * sizeof(float));
                }
            }
        }

        private static byte[] ByteToNormFloat(byte v)
        {
            float norm = ((float)v) / 255;
            return BitConverter.GetBytes(norm);
        }
    }
}
