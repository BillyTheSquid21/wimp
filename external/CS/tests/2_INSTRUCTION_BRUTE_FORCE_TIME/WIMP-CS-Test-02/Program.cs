using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using WimpCS;
using System.Runtime.InteropServices;

namespace WIMP_CS_Test_02
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

            //Read the args in the C way
            //We also check i + 1 < argc to ensure next argument was specified
            for (int i = 0; i < args.Length; ++i)
            {
                if (args[i] == "--master-port" && i + 1 < args.Length)
                {
                    master_port = Int32.Parse(args[i+1]);
                }
                else if (args[i] == "--process-port" && i + 1 < args.Length)
                {
                    process_port = Int32.Parse(args[i + 1]);
                }
            }

            Wimp.Init();
            WimpServer.InitLocalServer("client", "127.0.0.1", process_port);

            //Start reciever
            WimpReciever reciever = new WimpReciever("master", "127.0.0.1", master_port);
            reciever.Start("client", "127.0.0.1", process_port);

            //Add process
            WimpServer.AddProcess(reciever, WimpCore.WimpRelation.Parent);

            //Send the instructions
            //Instruction 1 - This sends the short instruction - in one go
            WimpLog.Log("Sending %d short instructions...\n", Program.SHORT_INSTR_COUNT);
            for (int i = 0; i < Program.SHORT_INSTR_COUNT; ++i)
            {
                WimpServer.AddInstruction("master", (UInt64)Program.Instructions.INCR_SHORT, (IntPtr)0, 0);
                if (i % (Program.SHORT_INSTR_COUNT / 10) == 0)
                {
                    float pc = (float)i / (float)Program.SHORT_INSTR_COUNT;
                    pc *= 100.0f;

                    WimpLog.Log("%f pc added!\n", pc);
                }
            }
            WimpLog.Log("Done!\n");

            //This tells the server to send off the instructions
            WimpLog.Log("Sending...\n");
            WimpServer.SendInstructions();
            WimpLog.Log("Done!\n");

            //Instruction 2 - This sends the long instruction - in one go
            int[] long_instr_args = Enumerable.Repeat<int>(64, 64).ToArray();
            IntPtr long_instr_args_ptr = Marshal.AllocHGlobal(long_instr_args.Length * sizeof(int));
            Marshal.Copy(long_instr_args, 0, long_instr_args_ptr, long_instr_args.Length);

            WimpLog.Log("Sending %d long instructions...\n", Program.LONG_INSTR_COUNT);
            for (int i = 0; i < Program.LONG_INSTR_COUNT; ++i)
            {
                WimpServer.AddInstruction("master", (UInt64)Program.Instructions.INCR_LONG, long_instr_args_ptr, 64 * sizeof(int));
                if (i % (Program.LONG_INSTR_COUNT / 10) == 0)
                {
                    float pc = (float)i / (float)Program.LONG_INSTR_COUNT;
                    pc *= 100.0f;

                    WimpLog.Log("%f pc added!\n", pc);
                }
            }
            Marshal.FreeHGlobal(long_instr_args_ptr);
            WimpLog.Log("Done!\n");

            //This tells the server to send off the instructions
            WimpLog.Log("Sending...\n");
            WimpServer.SendInstructions();
            WimpLog.Log("Done!\n");

            //Instruction 3 - This sends the short instruction - in batches of 1000 of the total
            WimpLog.Log("Sending %d short instructions in batch...\n", Program.SHORT_INSTR_COUNT);
            for (int i = 0; i < Program.SHORT_INSTR_COUNT; ++i)
            {
                WimpServer.AddInstruction("master", (UInt64)Program.Instructions.INCR_SHORT_BATCH, (IntPtr)0, 0);
                if (i % (Program.SHORT_INSTR_COUNT / 10) == 0)
                {
                    float pc = (float)i / (float)Program.SHORT_INSTR_COUNT;
                    pc *= 100.0f;

                    WimpLog.Log("%f pc added!\n", pc);
                }
                if (i % 1000 == 0)
                {
                    WimpServer.SendInstructions();
                }
            }
            WimpLog.Log("Done!\n");

            WimpLog.Log("Sending...\n");
            WimpServer.SendInstructions();
            WimpLog.Log("Done!\n");

            //Instruction 4 - This sends the long instruction - in batches of 1000 of the total
            WimpLog.Log("Sending %d long instructions in batch...\n", Program.LONG_INSTR_COUNT);
            for (int i = 0; i < Program.LONG_INSTR_COUNT; ++i)
            {
                WimpServer.AddInstruction("master", (UInt64)Program.Instructions.INCR_LONG_BATCH, (IntPtr)0, 0);
                if (i % (Program.LONG_INSTR_COUNT / 10) == 0)
                {
                    float pc = (float)i / (float)Program.LONG_INSTR_COUNT;
                    pc *= 100.0f;

                    WimpLog.Log("%f pc added!\n", pc);
                }
                if (i % 1000 == 0)
                {
                    WimpServer.SendInstructions();
                }
            }
            WimpLog.Log("Done!\n");

            WimpLog.Log("Sending...\n");
            WimpServer.SendInstructions();
            WimpLog.Log("Done!\n");

            WimpServer.AddInstruction("master", (UInt64)WimpCore.WIMPInstructionsCore.EXIT, (IntPtr)0, 0);

            WimpServer.SendInstructions();
            Thread.Sleep(1000);

            WimpServer.CloseLocalServer();
            Wimp.Shutdown();
        }
    }

    class Program
    {
        public enum Instructions : UInt64
        {
            INCR_SHORT = 0,
            INCR_LONG = 1,
            INCR_SHORT_BATCH = 2,
            INCR_LONG_BATCH = 3,
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
            "SHORT INSTRUCTIONS ARRIVED",
            "LONG INSTRUCTIONS ARRIVED",
            "SHORT BATCH ARRIVED",
            "LONG BATCH ARRIVED"
        };

        public enum TEST_ENUMS
        {
            STEP_PROCESS_VALIDATION,
            SHORT_INSTRUCTIONS_ARRIVED,
            LONG_INSTRUCTIONS_ARRIVED,
            SHORT_BATCH_ARRIVED,
            LONG_BATCH_ARRIVED
        };

        public const int SHORT_INSTR_COUNT = 50000;
        public const int LONG_INSTR_COUNT = 10000;

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

            //Init WIMP and the local server
            Wimp.Init();
            WimpServer.InitLocalServer("master", "127.0.0.1");

            //Assign child port
            String childPort = WimpServer.GetUnusedLocalPort().ToString();
            String parentPort = WimpServer.Port().ToString();

            string[] childArgs = { "--master-port", parentPort, "--process-port", childPort, };

            Thread childThread = new Thread(() => ChildProgram.Run(childArgs));
            childThread.Start();

            //Start reciever
            WimpReciever reciever = new WimpReciever("client", "127.0.0.1", Int32.Parse(childPort));
            reciever.Start("master", "127.0.0.1", Int32.Parse(parentPort));

            //Add process
            WimpServer.AddProcess(reciever, WimpCore.WimpRelation.Child);

            if (WimpServer.CheckProcessListening("client"))
            {
                WimpLog.Success("Process validated!\n");
                Steps[(int)TEST_ENUMS.STEP_PROCESS_VALIDATION].Status = true;
            }

            //Loop
            bool disconnect = false;
            int inc_sht_counter = 0;
            int inc_lng_counter = 0;
            int inc_sht_btch_counter = 0;
            int inc_lng_btch_counter = 0;
            while (!disconnect)
            {
                WimpServer.Lock();
                WimpInstructionNode currentnode = WimpServer.NextInstruction();
                while(!currentnode.IsNull())
                {
                    WimpInstructionNode.WimpInstrMeta meta = currentnode.GetMeta();
                    switch (meta.Instruction())
                    {
                        case (UInt64)Instructions.INCR_SHORT:
                            if (inc_sht_counter % (SHORT_INSTR_COUNT / 10) == 0)
                            {
                                float pc = (float)inc_sht_counter / (float)SHORT_INSTR_COUNT;
                                pc *= 100.0f;

                                WimpLog.Log("%f pc short recieved!\n", pc);
                            }
                            inc_sht_counter++;

                            if (inc_sht_counter == SHORT_INSTR_COUNT)
                            {
                                Steps[(int)TEST_ENUMS.SHORT_INSTRUCTIONS_ARRIVED].Status = true;
                            }
                            break;
                        case (UInt64)Instructions.INCR_LONG:
                            if (inc_lng_counter % (LONG_INSTR_COUNT / 10) == 0)
                            {
                                float pc = (float)inc_lng_counter / (float)LONG_INSTR_COUNT;
                                pc *= 100.0f;

                                WimpLog.Log("%f pc long recieved!\n", pc);
                            }
                            inc_lng_counter++;

                            if (inc_lng_counter == LONG_INSTR_COUNT)
                            {
                                Steps[(int)TEST_ENUMS.LONG_INSTRUCTIONS_ARRIVED].Status = true;
                            }
                            break;
                        case (UInt64)Instructions.INCR_SHORT_BATCH:
                            if (inc_sht_btch_counter % (SHORT_INSTR_COUNT / 10) == 0)
                            {
                                float pc = (float)inc_sht_btch_counter / (float)SHORT_INSTR_COUNT;
                                pc *= 100.0f;

                                WimpLog.Log("%f pc short batch recieved!\n", pc);
                            }
                            inc_sht_btch_counter++;

                            if (inc_sht_btch_counter == SHORT_INSTR_COUNT)
                            {
                                Steps[(int)TEST_ENUMS.SHORT_BATCH_ARRIVED].Status = true;
                            }
                            break;
                        case (UInt64)Instructions.INCR_LONG_BATCH:
                            if (inc_lng_btch_counter % (LONG_INSTR_COUNT / 10) == 0)
                            {
                                float pc = (float)inc_lng_btch_counter / (float)LONG_INSTR_COUNT;
                                pc *= 100.0f;

                                WimpLog.Log("%f pc long batch recieved!\n", pc);
                            }
                            inc_lng_btch_counter++;

                            if (inc_lng_btch_counter == LONG_INSTR_COUNT)
                            {
                                Steps[(int)TEST_ENUMS.LONG_BATCH_ARRIVED].Status = true;
                            }
                            break;
                        case (UInt64)WimpCore.WIMPInstructionsCore.EXIT:
                            disconnect = true;
                            break;
                        case (UInt64)WimpCore.WIMPInstructionsCore.LOG:
                            WimpLog.Log(Marshal.PtrToStringAnsi(meta.Arguments()));
                            break;
                        default:
                            break;
                    }

                    currentnode = WimpServer.NextInstruction();
                }
                WimpServer.Unlock();
            }

            Thread.Sleep(1000);
            WimpServer.CloseLocalServer();
            Wimp.Shutdown();

            bool passed = true;
            foreach(var step in Steps)
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
