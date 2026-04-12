using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using WimpCS;
using System.Runtime.InteropServices;

namespace WIMP_CS_Test_03
{
    public class ChildProgram
    {
        public static void Run(string[] args)
        {
            //Default values in case no arguments supplied
            String process_domain = "127.0.0.1";
            Int32 process_port = 8001;
            String master_domain = "127.0.0.1";
            Int32 master_port = 8000;

            String processName = "client";

            //Read the args in the C way
            //We also check i + 1 < argc to ensure next argument was specified
            for (int i = 0; i < args.Length; ++i)
            {
                if (args[i] == "--master-port" && i + 1 < args.Length)
                {
                    master_port = Int32.Parse(args[i + 1]);
                }
                else if (args[i] == "--process-port" && i + 1 < args.Length)
                {
                    process_port = Int32.Parse(args[i + 1]);
                }
                else if (args[i] == "--process-name" && i + 1 < args.Length)
                {
                    processName = args[i + 1];
                }
            }

            WimpLog.Log("Client: %s\n", processName);

            //Create a server local to this thread 
            Wimp.Init();
            WimpServer.InitLocalServer(processName, "127.0.0.1", process_port);

            //Start a reciever thread for the master process that called this thread
            WimpReciever reciever = new WimpReciever("master", "127.0.0.1", master_port);
            reciever.Start();

            //Add the master process to the table for tracking
            WimpServer.AddProcess(reciever, WimpCore.WimpRelation.Parent);

            //Step 1. Send an instruction to the other client
            String otherClient = "";
            if (processName == "client1")
            {
                otherClient = "client2";
            }
            else
            {
                otherClient = "client1";
            }

            //Send the instr here
            WimpServer.AddInstruction(otherClient, (UInt64)Program.Instructions.INSTR_HELLO, (IntPtr)0, 0);
            WimpServer.AddInstruction(otherClient, (UInt64)WimpInstructionsCore.EXIT, (IntPtr)0, 0);

            Thread.Sleep(5000);
            WimpServer.SendInstructions();

            //Program loop here
            bool disconnect = false;
            while (!disconnect)
            {
                WimpServer.Lock();
                WimpInstructionNode currentnode = WimpServer.NextInstruction();
                while (!currentnode.IsNull())
                {
                    WimpInstructionNode.WimpInstrMeta meta = currentnode.GetMeta();
                    if (WimpServer.NeedsRouting(meta.Destination(), currentnode))
                    {
                        Console.WriteLine("Routing instruction to: " + meta.Destination());
                        currentnode = WimpServer.NextInstruction();
                        continue;
                    }

                    switch (meta.Instruction())
                    {
                        case (UInt64)Program.Instructions.INSTR_HELLO:
                            WimpLog.Log("Hello! Recieved from: %s\n", meta.Source());
                            if (otherClient == meta.Source())
                            {
                                if (meta.Destination() == "client1")
                                {
                                    Program.Steps[(int)Program.TEST_ENUMS.C1_RECIEVESFROM_C2].Status = true;
                                }
                                else if (meta.Destination() == "client2")
                                {
                                    Program.Steps[(int)Program.TEST_ENUMS.C2_RECIEVESFROM_C1].Status = true;
                                }
                            }
                            break;
                        case (UInt64)WimpInstructionsCore.EXIT:
                            WimpLog.Log("Exiting!\n");
                            disconnect = true;
                            break;
                        default:
                            break;
                    }

                    currentnode = WimpServer.NextInstruction();
                }
                WimpServer.Unlock();
            }

            Thread.Sleep(1000);
            WimpServer.AddInstruction("master", (UInt64)WimpInstructionsCore.EXIT, (IntPtr)0, 0);
            WimpServer.SendInstructions();

            WimpLog.Log("Client thread closed\n");
            WimpServer.CloseLocalServer();
            Wimp.Shutdown();
        }
    }

    class Program
    {
        public enum Instructions : UInt64
        {
            INSTR_HELLO = 0
        }

        public struct PassMat
        {
            public PassMat(String name)
            {
                StepName = name;
                Status = false;
            }

            public static implicit operator PassMat(string name)
            {
                return new PassMat() { StepName = name, Status = false };
            }

            public String StepName;
            public bool Status;
        }

        public static PassMat[] Steps = {
            "PROCESS VALIDATION",
            "CHILD 1 RECEIVES FROM CHILD 2",
            "CHILD 2 RECEIVES FROM CHILD 1"
        };

        public enum TEST_ENUMS
        {
            STEP_PROCESS_VALIDATION = 0,
            C1_RECIEVESFROM_C2 = 1,
            C2_RECIEVESFROM_C1 = 2,
        };

        /// 
        /// Ensures the console prints in color
        /// 
        const int STD_OUTPUT_HANDLE = -11;
        const uint ENABLE_VIRTUAL_TERMINAL_PROCESSING = 4;

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern IntPtr GetStdHandle(int nStdHandle);

        [DllImport("kernel32.dll")]
        static extern bool GetConsoleMode(IntPtr hConsoleHandle, out uint lpMode);

        [DllImport("kernel32.dll")]
        static extern bool SetConsoleMode(IntPtr hConsoleHandle, uint dwMode);
        static void Main(string[] args)
        {

            var handle = GetStdHandle(STD_OUTPUT_HANDLE);
            uint mode;
            GetConsoleMode(handle, out mode);
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(handle, mode);

            //Initialize the socket library
            Wimp.Init();

            //Get unused random ports for the master and end process to run on
            String masterPort = WimpServer.GetUnusedLocalPort().ToString();
            String client1Port = WimpServer.GetUnusedLocalPort().ToString();
            String client2Port = WimpServer.GetUnusedLocalPort().ToString();

            //Start a local server for the master process
            WimpServer.InitLocalServer("master", "127.0.0.1", Int32.Parse(masterPort));

            //Start the client processes, creating the command line arguments and creating a new thread
            string[] entry1 = { "--master-port", masterPort, "--process-port", client1Port, "--process-name", "client1" };
            WimpProcess client1 = new WimpProcess();
            client1.Start("client1", entry1, ChildProgram.Run);

            //Give child threads time to start and initialize their servers
            Thread.Sleep(500);

            //Start a reciever thread for the client processes that the master started
            //Currently only support adding one process at a time
            WimpReciever reciever1 = new WimpReciever("client1", "127.0.0.1", Int32.Parse(client1Port));
            reciever1.Start();
            WimpServer.AddProcess(reciever1, WimpCore.WimpRelation.Child);

            string[] entry2 = { "--master-port", masterPort, "--process-port", client2Port, "--process-name", "client2" };
            WimpProcess client2 = new WimpProcess();
            client2.Start("client2", entry2, ChildProgram.Run);

            Thread.Sleep(500);

            WimpReciever reciever2 = new WimpReciever("client2", "127.0.0.1", Int32.Parse(client2Port));
            reciever2.Start();
            WimpServer.AddProcess(reciever2, WimpCore.WimpRelation.Child);

            if (WimpServer.CheckProcessListening("client1") && WimpServer.CheckProcessListening("client2"))
            {
                WimpLog.Success("Process validated!\n");
                Steps[(int)TEST_ENUMS.STEP_PROCESS_VALIDATION].Status = true;
            }
            else
            {
                WimpLog.Fail("Process couldn't be validated!\n");
            }

            //Perform the two loops - read incoming, then send outgoing
            bool disconnect = false;
            while (!disconnect)
            {
                WimpServer.Lock();
                WimpInstructionNode currentnode = WimpServer.NextInstruction();
                while (!currentnode.IsNull())
                {
                    WimpInstructionNode.WimpInstrMeta meta = currentnode.GetMeta();
                    if (WimpServer.NeedsRouting(meta.Destination(), currentnode))
                    {
                        currentnode = WimpServer.NextInstruction();
                        continue;
                    }

                    switch (meta.Instruction())
                    {
                        case (UInt64)WimpInstructionsCore.LOG:
                            WimpLog.Log("%s\n", Marshal.PtrToStringAnsi(meta.Arguments()));
                            break;
                        case (UInt64)WimpInstructionsCore.EXIT:
                            WimpLog.Log("Exiting!\n");
                            disconnect = true;
                            break;
                        default:
                            break;
                    }

                    currentnode = WimpServer.NextInstruction();
                }
                WimpServer.Unlock();

                //Send outgoing
                WimpServer.SendInstructions();
            }

            Thread.Sleep(1000);
            WimpServer.CloseLocalServer();
            Wimp.Shutdown();

            bool passed = true;
            foreach (var step in Steps)
            {
                Console.WriteLine(step.StepName + ": " + step.Status.ToString());
                passed &= step.Status;
            }

            if (passed)
            {
                Console.WriteLine("Test passed!");
            }
            else
            {
                Console.WriteLine("Test failed!");
            }
            Thread.Sleep(5000);
        }
    }
}
