using System;
using System.Drawing;

class Program
{
    static void Main()
    {
        string path = @"C:\Users\JZX\.gemini\antigravity\brain\f17ffbb2-3f4d-49a3-992c-f867052befd2\media__1776622235163.png";
        try {
            using (Bitmap bmp = new Bitmap(path))
            {
                Console.WriteLine(string.Format("Size: {0}x{1}", bmp.Width, bmp.Height));
                
                // Print a small ASCII representation
                int asciiW = 60;
                int asciiH = 30;
                for (int y = 0; y < asciiH; y++)
                {
                    for (int x = 0; x < asciiW; x++)
                    {
                        int bx = x * bmp.Width / asciiW;
                        int by = y * bmp.Height / asciiH;
                        Color c = bmp.GetPixel(bx, by);
                        int brightness = (c.R + c.G + c.B) / 3;
                        
                        char sym = ' ';
                        if (brightness < 50) sym = '#';
                        else if (brightness < 100) sym = '%';
                        else if (brightness < 150) sym = '*';
                        else if (brightness < 200) sym = '+';
                        else sym = '.';
                        
                        Console.Write(sym);
                    }
                    Console.WriteLine();
                }

                // Sample some colors
                Console.WriteLine("Center row colors:");
                for (int x = 0; x < bmp.Width; x += bmp.Width / 10)
                {
                    Color c = bmp.GetPixel(x, bmp.Height / 2);
                    Console.WriteLine(string.Format("X={0}: R={1} G={2} B={3}", x, c.R, c.G, c.B));
                }
            }
        } catch (Exception ex) {
            Console.WriteLine(ex.Message);
        }
    }
}
