using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace ColorUtils
{
    public class LabColor
    {
        public int l;
        public int a;
        public int b;

        public LabColor(int l, int a, int b)
        {
            this.l = l;
            this.a = a;
            this.b = b;
        }

        public static int toRGB(LabColor c)
        {
            return 0;
        }

        public static int toRGB(int l, int a, int b)
        {
            return 0;
        }

        public static LabColor fromRGB(int rgb)
        {
            return new LabColor(0, 0, 0);
        }

        public static LabColor fromRGB(int r, int g, int b)
        {
            return new LabColor(0, 0, 0);
        }

        public static LabColor operator +(LabColor c1, LabColor c2)
        {
            return new LabColor(c1.l + c2.l, c1.a + c2.a, c1.b + c2.b);
        }

        public static LabColor operator -(LabColor c1, LabColor c2)
        {
            return new LabColor(c1.l - c2.l, c1.a - c2.a, c1.b - c2.b);
        }
    }

    public class RGBColor
    {
        public int r;
        public int g;
        public int b;

        public RGBColor(int r, int g, int b)
        {
            this.r = r;
            this.g = g;
            this.b = b;
        }

        public RGBColor(int rgb)
        {
            this.r = rgb & 0xFF0000;
            this.g = rgb & 0xFF00;
            this.b = rgb & 0xFF;
        }
    }

    public class XYZColor
    {
        public double x;
        public double y;
        public double z;

        public XYZColor(double x, double y, double z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }
    }

    public class ColorConversor
    {
        public static XYZColor rgb2xyz(RGBColor rgb)
        {
            double r = rgb.r / 255.0;
            double g = rgb.g / 255.0;
            double b = rgb.b / 255.0;

            if (r > 0.04045) { r = Math.Pow((r + 0.055) / 1.055, 2.4); }
            else { r = r / 12.92; }
            if (g > 0.04045) { g = Math.Pow((g + 0.055) / 1.055, 2.4); }
            else { g = g / 12.92; }
            if (b > 0.04045) { b = Math.Pow((b + 0.055) / 1.055, 2.4); }
            else { b = b / 12.92; }
            r = r * 100;
            g = g * 100;
            b = b * 100;

            //Observer. = 2°, Illuminant = D65
            XYZColor xyz = new XYZColor(0, 0, 0);
            /*
            xyz.x = r * 0.4124 + g * 0.3576 + b * 0.1805;
            xyz.y = r * 0.2126 + g * 0.7152 + b * 0.0722;
            xyz.z = r * 0.0193 + g * 0.1192 + b * 0.9505;
             */

            xyz.x = r * 0.4124564 + g * 0.3575761 + b * 0.1804375;
            xyz.y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750;
            xyz.z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041;

            return xyz;
        }

        public static LabColor xyz2lab(XYZColor xyz)
        {
            const double REF_X = 95.047; // Observer= 2°, Illuminant= D65
            const double REF_Y = 100.000;
            const double REF_Z = 108.883;
            double x = xyz.x / REF_X;
            double y = xyz.y / REF_Y;
            double z = xyz.z / REF_Z;

            if (x > 0.008856) { x = Math.Pow(x, 1.0 / 3.0); }
            else { x = (7.787 * x) + (16.0 / 116.0); }
            if (y > 0.008856) { y = Math.Pow(y, 1.0 / 3.0); }
            else { y = (7.787 * y) + (16.0 / 116.0); }
            if (z > 0.008856) { z = Math.Pow(z, 1.0 / 3.0); }
            else { z = (7.787 * z) + (16.0 / 116.0); }

            LabColor lab = new LabColor(0, 0, 0);
            lab.l = (int)(116 * y) - 16;
            lab.a = (int)(500 * (x - y));
            lab.b = (int)(200 * (y - z));

            return lab;
        }

        public static XYZColor lab2xyz(LabColor lab)
        {
            const double REF_X = 95.047; // Observer= 2°, Illuminant= D65
            const double REF_Y = 100.000;
            const double REF_Z = 108.883;
            double y = (lab.l + 16.0) / 116.0;
            double x = lab.a / 500.0 + y;
            double z = y - lab.b / 200.0;

            if (Math.Pow(y, 3) > 0.008856) { y = Math.Pow(y, 3); }
            else { y = (y - 16.0 / 116.0) / 7.787; }
            if (Math.Pow(x, 3) > 0.008856) { x = Math.Pow(x, 3); }
            else { x = (x - 16.0 / 116.0) / 7.787; }
            if (Math.Pow(z, 3) > 0.008856) { z = Math.Pow(z, 3); }
            else { z = (z - 16.0 / 116.0) / 7.787; }

            XYZColor xyz = new XYZColor(0, 0, 0);
            xyz.x = REF_X * x;
            xyz.y = REF_Y * y;
            xyz.z = REF_Z * z;

            return xyz;
        }

        public static RGBColor xyz2rgb(XYZColor xyz)
        {
            //X from 0 to  95.047      (Observer = 2°, Illuminant = D65)
            //Y from 0 to 100.000
            //Z from 0 to 108.883
            double x = xyz.x / 100.0;
            double y = xyz.y / 100.0;
            double z = xyz.z / 100.0;

            /*
            double r = x * 3.2406 + y * -1.5372 + z * -0.4986;
            double g = x * -0.9689 + y * 1.8758 + z * 0.0415;
            double b = x * 0.0557 + y * -0.2040 + z * 1.0570;
             */
            double r = x * 3.2404542 + y * -1.5371385 + z * -0.4985314;
            double g = x * -0.9692660 + y * 1.8760108 + z * 0.0415560;
            double b = x * 0.0556434 + y * -0.2040259 + z * 1.0572252;

            if (r > 0.0031308) { r = 1.055 * Math.Pow(r, (1 / 2.4)) - 0.055; }
            else { r = 12.92 * r; }
            if (g > 0.0031308) { g = 1.055 * Math.Pow(g, (1 / 2.4)) - 0.055; }
            else { g = 12.92 * g; }
            if (b > 0.0031308) { b = 1.055 * Math.Pow(b, (1 / 2.4)) - 0.055; }
            else { b = 12.92 * b; }

            RGBColor rgb = new RGBColor(0, 0, 0);
            rgb.r = (int)Math.Round(r * 255);
            rgb.g = (int)Math.Round(g * 255);
            rgb.b = (int)Math.Round(b * 255);

            return rgb;
        }

        public static LabColor rgb2lab(RGBColor rgb)
        {
            XYZColor xyz = ColorConversor.rgb2xyz(rgb);
            return ColorConversor.xyz2lab(xyz);
        }

        public static RGBColor lab2rgb(LabColor lab)
        {
            XYZColor xyz = ColorConversor.lab2xyz(lab);
            return ColorConversor.xyz2rgb(xyz);
        }
    }
}
