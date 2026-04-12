using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

using static WimpCS.WimpCore;

namespace WimpCS
{
    public class WimpProcess
    {
        private bool m_IsRunning = false;
        public String Name { get; private set; }

        public WimpProcess()
        {
            this.Name = "";
        }

        public WimpProcessResult Start(string name, string[] args, Action<string[]> action)
        {
            if (m_IsRunning)
            {
                Console.WriteLine("Process is already running!");
                return WimpProcessResult.FAIL;
            }

            Thread childThread = new Thread(() => action(args));
            childThread.Start();

            m_IsRunning = true;
            this.Name = name;
            return WimpProcessResult.SUCCESS;
        }

        public WimpProcessResult Start(string name, string[] args, string executablePath)
        {
            if (m_IsRunning)
            {
                Console.WriteLine("Process is already running!");
                return WimpProcessResult.FAIL;
            }
            
            // C String marshalling done manually to avoid corruption
            // A bit amateurish but it actually works which is good
            IntPtr entry = (IntPtr)0;
            switch (args.Length)
            {
                case 0:
                    entry = WimpCore.wimp_get_entry(0);
                    break;
                case 1:
                    entry = WimpCore.wimp_get_entry(1, args[0]);
                    break;
                case 2:
                    entry = WimpCore.wimp_get_entry(2, args[0], args[1]);
                    break;
                case 3:
                    entry = WimpCore.wimp_get_entry(3, args[0], args[1], args[2]);
                    break;
                case 4:
                    entry = WimpCore.wimp_get_entry(4, args[0], args[1], args[2], args[3]);
                    break;
                case 5:
                    entry = WimpCore.wimp_get_entry(5, args[0], args[1], args[2], args[3], args[4]);
                    break;
                case 6:
                    entry = WimpCore.wimp_get_entry(6, args[0], args[1], args[2], args[3], args[4], args[5]);
                    break;
                default:
                    Console.WriteLine("Too many arguments! Max is 6.");
                    return WimpProcessResult.FAIL;
            }

            WimpProcessResult result = (WimpProcessResult)WimpCore.wimp_start_executable_process(name, executablePath, entry);
            if (result == WimpProcessResult.SUCCESS)
            {
                m_IsRunning = true;
                this.Name = name;
            }

            WimpCore.wimp_free_entry(entry);
            return result;
        }
    }
}
