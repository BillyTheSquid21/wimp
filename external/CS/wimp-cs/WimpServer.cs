using System;
using System.Collections.Generic;
using System.Threading;

using static WimpCS.WimpCore;

namespace WimpCS
{
    public class WimpServer
    { 
        private static ThreadLocal<Int32> s_Port = new ThreadLocal<Int32>(() =>
        {
            return 8000;
        });

        private static ThreadLocal<string> s_Domain = new ThreadLocal<string>(() =>
        {
            return "127.0.0.1";
        });

        private static ThreadLocal<IntPtr> s_ServerPtr = new ThreadLocal<IntPtr>(() =>
        {
            return (IntPtr)0;
        });

        private static ThreadLocal<List<WimpReciever>> s_Processes = new ThreadLocal<List<WimpReciever>>(() =>
        {
            return new List<WimpReciever>();
        });

        public static WimpServerResult InitLocalServer(String processName, String domain, Int32 port)
        {
            WimpServerResult res = (WimpServerResult)wimp_init_local_server(processName, domain, port);
            s_Port.Value = port;
            s_Domain.Value = domain;
            s_ServerPtr.Value = wimp_get_local_server();
            return res;
        }

        public static WimpServerResult InitLocalServer(String processName, String domain)
        {
            Int32 port = wimp_assign_unused_local_port();
            return InitLocalServer(processName, domain, port);
        }

        public static void CloseLocalServer()
        {
            wimp_close_local_server();
            s_Port.Value = 8000;
            s_Domain.Value = "127.0.0.1";
            s_ServerPtr.Value = (IntPtr)0;
            s_Processes.Value.Clear();
        }

        public static WimpProcessResult AddProcess(WimpReciever reciever, WimpRelation relation)
        {
            WimpProcessResult res = (WimpProcessResult)wimp_add_local_server_process(reciever.Name(), reciever.Domain(), reciever.Port(), (Int32)relation);
            if (res == WimpProcessResult.SUCCESS)
            {
                s_Processes.Value.Add(reciever);
            }
            return res;
        }

        public static bool CheckProcessListening(String process)
        {
            return wimp_server_check_process_listening(s_ServerPtr.Value, process);
        }

        public static void AddInstruction(String destination, UInt64 instr, IntPtr args, UInt64 argSize)
        {
            wimp_add_local_server(destination, instr, args, argSize);
        }

        public static void SendInstructions()
        {
            wimp_server_send_instructions(s_ServerPtr.Value);
        }

        public static Int32 Port()
        {
            return s_Port.Value;
        }

        public static Int32 GetUnusedLocalPort()
        {
            return wimp_assign_unused_local_port();
        }

        public static WimpInstructionNode NextInstruction()
        {
            IntPtr handle = WimpCore.wimp_incoming_queue_local_server_pop();
            return new WimpInstructionNode(handle);
        }

        public static void Lock()
        {
            WimpCore.wimp_incoming_queue_local_server_lock();
        }

        public static void Unlock()
        {
            WimpCore.wimp_incoming_queue_local_server_unlock();
        }
    }
}
