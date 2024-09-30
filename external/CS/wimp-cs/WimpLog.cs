using System;
using System.Runtime.InteropServices;
using System.Threading;

using static WimpCS.WimpCore;

namespace WimpCS
{
    public class WimpLog
    {
        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1, Int32 i2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1, Int32 i2, Int32 i3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1, [MarshalAs(UnmanagedType.LPStr)] String s2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1, [MarshalAs(UnmanagedType.LPStr)] String s2, [MarshalAs(UnmanagedType.LPStr)] String s3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, float f1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, float f1, float f2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, float f1, float f2, float f3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1, Int32 i2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1, Int32 i2, Int32 i3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1, [MarshalAs(UnmanagedType.LPStr)] String s2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1, [MarshalAs(UnmanagedType.LPStr)] String s2, [MarshalAs(UnmanagedType.LPStr)] String s3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, float f1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, float f1, float f2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, float f1, float f2, float f3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1, Int32 i2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1, Int32 i2, Int32 i3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1, [MarshalAs(UnmanagedType.LPStr)] String s2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1, [MarshalAs(UnmanagedType.LPStr)] String s2, [MarshalAs(UnmanagedType.LPStr)] String s3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, float f1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, float f1, float f2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, float f1, float f2, float f3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1, Int32 i2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, Int32 i1, Int32 i2, Int32 i3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1, [MarshalAs(UnmanagedType.LPStr)] String s2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, [MarshalAs(UnmanagedType.LPStr)] String s1, [MarshalAs(UnmanagedType.LPStr)] String s2, [MarshalAs(UnmanagedType.LPStr)] String s3);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, float f1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, float f1, float f2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, float f1, float f2, float f3);

        public static void Log([MarshalAs(UnmanagedType.LPStr)] String format)
        {
            wimp_log(format);
        }

        public static void Log([MarshalAs(UnmanagedType.LPStr)] String format, params String[] args)
        {
            switch (args.Length)
            {
            case 0:
                wimp_log(format);
                break;
            case 1:
                wimp_log(format, args[0]);
                break;
            case 2:
                wimp_log(format, args[0], args[1]);
                break;
            case 3:
                wimp_log(format, args[0], args[1], args[2]);
                break;
            default:
                break;
            }
        }

        public static void Log([MarshalAs(UnmanagedType.LPStr)] String format, params Int32[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log(format);
                    break;
                case 1:
                    wimp_log(format, args[0]);
                    break;
                case 2:
                    wimp_log(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Log([MarshalAs(UnmanagedType.LPStr)] String format, params float[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log(format);
                    break;
                case 1:
                    wimp_log(format, args[0]);
                    break;
                case 2:
                    wimp_log(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Success([MarshalAs(UnmanagedType.LPStr)] String format, params String[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_success(format);
                    break;
                case 1:
                    wimp_log_success(format, args[0]);
                    break;
                case 2:
                    wimp_log_success(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_success(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Success([MarshalAs(UnmanagedType.LPStr)] String format, params Int32[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_success(format);
                    break;
                case 1:
                    wimp_log_success(format, args[0]);
                    break;
                case 2:
                    wimp_log_success(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_success(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Success([MarshalAs(UnmanagedType.LPStr)] String format, params float[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_success(format);
                    break;
                case 1:
                    wimp_log_success(format, args[0]);
                    break;
                case 2:
                    wimp_log_success(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_success(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Fail([MarshalAs(UnmanagedType.LPStr)] String format, params String[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_fail(format);
                    break;
                case 1:
                    wimp_log_fail(format, args[0]);
                    break;
                case 2:
                    wimp_log_fail(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_fail(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Fail([MarshalAs(UnmanagedType.LPStr)] String format, params Int32[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_fail(format);
                    break;
                case 1:
                    wimp_log_fail(format, args[0]);
                    break;
                case 2:
                    wimp_log_fail(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_fail(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Fail([MarshalAs(UnmanagedType.LPStr)] String format, params float[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_fail(format);
                    break;
                case 1:
                    wimp_log_fail(format, args[0]);
                    break;
                case 2:
                    wimp_log_fail(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_fail(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Important([MarshalAs(UnmanagedType.LPStr)] String format, params String[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_important(format);
                    break;
                case 1:
                    wimp_log_important(format, args[0]);
                    break;
                case 2:
                    wimp_log_important(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_important(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Important([MarshalAs(UnmanagedType.LPStr)] String format, params Int32[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_important(format);
                    break;
                case 1:
                    wimp_log_important(format, args[0]);
                    break;
                case 2:
                    wimp_log_important(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_important(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }

        public static void Important([MarshalAs(UnmanagedType.LPStr)] String format, params float[] args)
        {
            switch (args.Length)
            {
                case 0:
                    wimp_log_important(format);
                    break;
                case 1:
                    wimp_log_important(format, args[0]);
                    break;
                case 2:
                    wimp_log_important(format, args[0], args[1]);
                    break;
                case 3:
                    wimp_log_important(format, args[0], args[1], args[2]);
                    break;
                default:
                    break;
            }
        }
    }
}
