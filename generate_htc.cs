using System;
using System.Drawing;
using System.Drawing.Drawing2D;
using System.Drawing.Text;
using System.IO;

namespace HtcGenerator
{
    class Program
    {
        const int CARD_W = 96;
        const int CARD_H = 110;
        const int HALF_W = 48;
        const int CORNER_RAD = 8;
        
        static Color BG_COLOR_TOP1 = Color.FromArgb(245, 245, 245);
        static Color BG_COLOR_TOP2 = Color.FromArgb(205, 205, 205); // slightly darker bot of top-half
        static Color BG_COLOR_BOT1 = Color.FromArgb(190, 190, 190);
        static Color BG_COLOR_BOT2 = Color.FromArgb(230, 230, 230);
        static Color TEXT_COLOR = Color.FromArgb(24, 28, 27);
        static Color LINE_COLOR = Color.FromArgb(20, 20, 20);
        static Color BG_FRAME = Color.Black; // Blend with black for kColorBg

        static void Main(string[] args)
        {
            Console.WriteLine("Generating HTC UI Left/Right halves v2...");
            using (StreamWriter sw = new StreamWriter("Htc_Clock_Data.h"))
            {
                sw.WriteLine("#ifndef HTC_CLOCK_DATA_H");
                sw.WriteLine("#define HTC_CLOCK_DATA_H");
                sw.WriteLine("#include <Arduino.h>");
                sw.WriteLine(string.Format("#define CLOCK_HALF_W {0}", HALF_W));
                sw.WriteLine(string.Format("#define CLOCK_H {0}\n", CARD_H));

                GenerateArrays(sw, true);
                GenerateArrays(sw, false);

                sw.WriteLine("const uint16_t* const digits_left[10] = {");
                for (int i = 0; i < 10; i++) sw.Write(string.Format("digit_left_{0}", i) + (i < 9 ? ", " : ""));
                sw.WriteLine("};\n");

                sw.WriteLine("const uint16_t* const digits_right[10] = {");
                for (int i = 0; i < 10; i++) sw.Write(string.Format("digit_right_{0}", i) + (i < 9 ? ", " : ""));
                sw.WriteLine("};\n");

                sw.WriteLine("#endif");
            }
            Console.WriteLine("Generation successful.");
        }

        static void GenerateArrays(StreamWriter sw, bool isLeft)
        {
            string prefix = isLeft ? "left" : "right";
            for (int digit = 0; digit < 10; digit++)
            {
                Bitmap bmp = RenderHalf(digit, isLeft);
                sw.WriteLine(string.Format("const uint16_t digit_{0}_{1}[] PROGMEM = {{", prefix, digit));
                sw.Write("    ");
                int cnt = 0;
                for (int y = 0; y < bmp.Height; y++)
                {
                    for (int x = 0; x < bmp.Width; x++)
                    {
                        Color p = bmp.GetPixel(x, y);
                        
                        float a = p.A / 255f;
                        int r = (int)(p.R * a + BG_FRAME.R * (1 - a));
                        int g = (int)(p.G * a + BG_FRAME.G * (1 - a));
                        int b = (int)(p.B * a + BG_FRAME.B * (1 - a));
                        
                        int c565 = ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
                        sw.Write(string.Format("0x{0:X4}, ", c565));
                        cnt++;
                        if (cnt % 20 == 0) sw.Write("\n    ");
                    }
                }
                sw.WriteLine("\n};\n");
                bmp.Dispose();
            }
        }

        static Bitmap RenderHalf(int digit, bool isLeft)
        {
            int scale = 2;
            Bitmap fullBmp = new Bitmap(CARD_W * scale, CARD_H * scale);
            using (Graphics g = Graphics.FromImage(fullBmp))
            {
                g.SmoothingMode = SmoothingMode.AntiAlias;
                g.TextRenderingHint = TextRenderingHint.AntiAliasGridFit;
                g.InterpolationMode = InterpolationMode.HighQualityBicubic;
                g.Clear(Color.Transparent);

                int rad = CORNER_RAD * scale;
                GraphicsPath path = new GraphicsPath();
                path.AddArc(0, 0, rad * 2, rad * 2, 180, 90);
                path.AddArc(fullBmp.Width - rad * 2 - 1, 0, rad * 2, rad * 2, 270, 90);
                path.AddArc(fullBmp.Width - rad * 2 - 1, fullBmp.Height - rad * 2 - 1, rad * 2, rad * 2, 0, 90);
                path.AddArc(0, fullBmp.Height - rad * 2 - 1, rad * 2, rad * 2, 90, 90);
                path.CloseFigure();

                Region region = new Region(path);
                g.SetClip(region, CombineMode.Replace);

                Rectangle topRect = new Rectangle(0, 0, fullBmp.Width, fullBmp.Height / 2);
                using (LinearGradientBrush brTop = new LinearGradientBrush(topRect, BG_COLOR_TOP1, BG_COLOR_TOP2, LinearGradientMode.Vertical))
                {
                    g.FillRectangle(brTop, topRect);
                }

                Rectangle botRect = new Rectangle(0, fullBmp.Height / 2, fullBmp.Width, fullBmp.Height / 2);
                using (LinearGradientBrush brBot = new LinearGradientBrush(botRect, BG_COLOR_BOT1, BG_COLOR_BOT2, LinearGradientMode.Vertical))
                {
                    g.FillRectangle(brBot, botRect);
                }

                // Trebuchet MS or Arial Narrow looks more beautiful and blocky for Sense UI
                using (Font font = new Font("Trebuchet MS", 136, FontStyle.Bold))
                {
                    string text = digit.ToString();
                    SizeF size = g.MeasureString(text, font, new PointF(0, 0), StringFormat.GenericTypographic);
                    // Adjust spacing so it forms a pair seamlessly
                    // Left digit center slightly to the left, Right digit center slightly to the right to simulate "21"
                    float targetX_unscaled = isLeft ? 28 : 68; // closer together
                    float tx = (targetX_unscaled * scale) - (size.Width / 2f);
                    float ty = (fullBmp.Height - size.Height) / 2f - 4 * scale;
                    using (SolidBrush textBr = new SolidBrush(TEXT_COLOR))
                    {
                        g.DrawString(text, font, textBr, tx, ty, StringFormat.GenericTypographic);
                    }
                }

                using (SolidBrush lineBr = new SolidBrush(LINE_COLOR))
                {
                    g.FillRectangle(lineBr, 0, fullBmp.Height / 2 - 2, fullBmp.Width, 5);
                }
            }

            Bitmap scaled = new Bitmap(CARD_W, CARD_H);
            using (Graphics gScale = Graphics.FromImage(scaled))
            {
                gScale.InterpolationMode = InterpolationMode.HighQualityBicubic;
                gScale.DrawImage(fullBmp, 0, 0, CARD_W, CARD_H);
            }
            fullBmp.Dispose();

            Bitmap half = new Bitmap(HALF_W, CARD_H);
            using (Graphics gHalf = Graphics.FromImage(half))
            {
                Rectangle srcRect = isLeft ? new Rectangle(0, 0, HALF_W, CARD_H) : new Rectangle(HALF_W, 0, HALF_W, CARD_H);
                gHalf.DrawImage(scaled, 
                    new Rectangle(0, 0, HALF_W, CARD_H), 
                    srcRect, 
                    GraphicsUnit.Pixel);
            }
            scaled.Dispose();

            return half;
        }
    }
}
