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
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, Single f1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, Single f1, Single f2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log([MarshalAs(UnmanagedType.LPStr)] String format, Single f1, Single f2, Single f3);

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
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, Single f1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, Single f1, Single f2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_success([MarshalAs(UnmanagedType.LPStr)] String format, Single f1, Single f2, Single f3);

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
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, Single f1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, Single f1, Single f2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_fail([MarshalAs(UnmanagedType.LPStr)] String format, Single f1, Single f2, Single f3);

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
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, Single f1);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, Single f1, Single f2);

        [DllImport("wimp.dll")]
        private static extern void wimp_log_important([MarshalAs(UnmanagedType.LPStr)] String format, Single f1, Single f2, Single f3);

        ///
        /// Currently floats don't seem to correctly be interpreted
        /// Workaround is to convert to string and treat that way
        /// TODO: Fix
        /// 

        public static void Log([MarshalAs(UnmanagedType.LPStr)] String format)
        {
            wimp_log(format);
        }

        public static void Log([MarshalAs(UnmanagedType.LPStr)] String format, params String[] args)
        {
            switch (args.Length)
            {
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
            String format_str = format.Replace("%f", "%s");
            switch (args.Length)
            {
                case 1:
                    wimp_log(format_str, args[0].ToString());
                    break;
                case 2:
                    wimp_log(format_str, args[0].ToString(), args[1].ToString());
                    break;
                case 3:
                    wimp_log(format_str, args[0].ToString(), args[1].ToString(), args[2].ToString());
                    break;
                default:
                    break;
            }
        }

        public static void Success([MarshalAs(UnmanagedType.LPStr)] String format)
        {
            wimp_log_success(format);
        }

        public static void Success([MarshalAs(UnmanagedType.LPStr)] String format, params String[] args)
        {
            switch (args.Length)
            {
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
            String format_str = format.Replace("%f", "%s");
            switch (args.Length)
            {
                case 1:
                    wimp_log_success(format_str, args[0].ToString());
                    break;
                case 2:
                    wimp_log_success(format_str, args[0].ToString(), args[1].ToString());
                    break;
                case 3:
                    wimp_log_success(format_str, args[0].ToString(), args[1].ToString(), args[2].ToString());
                    break;
                default:
                    break;
            }
        }

        public static void Fail([MarshalAs(UnmanagedType.LPStr)] String format)
        {
            wimp_log_fail(format);
        }

        public static void Fail([MarshalAs(UnmanagedType.LPStr)] String format, params String[] args)
        {
            switch (args.Length)
            {
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
            String format_str = format.Replace("%f", "%s");
            switch (args.Length)
            {
                case 1:
                    wimp_log_fail(format_str, args[0].ToString());
                    break;
                case 2:
                    wimp_log_fail(format_str, args[0].ToString(), args[1].ToString());
                    break;
                case 3:
                    wimp_log_fail(format_str, args[0].ToString(), args[1].ToString(), args[2].ToString());
                    break;
                default:
                    break;
            }
        }

        public static void Important([MarshalAs(UnmanagedType.LPStr)] String format)
        {
            wimp_log_important(format);
        }

        public static void Important([MarshalAs(UnmanagedType.LPStr)] String format, params String[] args)
        {
            switch (args.Length)
            {
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
            String format_str = format.Replace("%f", "%s");
            switch (args.Length)
            {
                case 1:
                    wimp_log_important(format_str, args[0].ToString());
                    break;
                case 2:
                    wimp_log_important(format_str, args[0].ToString(), args[1].ToString());
                    break;
                case 3:
                    wimp_log_important(format_str, args[0].ToString(), args[1].ToString(), args[2].ToString());
                    break;
                default:
                    break;
            }
        }
    }
}
