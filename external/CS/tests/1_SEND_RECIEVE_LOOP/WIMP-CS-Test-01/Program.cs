using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using WimpCS;

namespace WIMP_CS_Test_01
{
    class Program
    {
        static void Main(string[] args)
        {
            Wimp.wimp_init();
            Thread.Sleep(10000);
            Wimp.wimp_shutdown();
        }
    }
}
