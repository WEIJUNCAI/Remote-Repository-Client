﻿/////////////////////////////////////////////////////////////////////////////
//  TestClient2.cs - Test client2 for demonstartion                        //
//  Language:     C#, VS 2015                                              //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai     										   //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the implementaion of test client to demonstrate
*   the functionalities supported by repository
*
*/
/*
*   Build Process
*   -------------
*   - Required files: CLIshim.h, CLIshim.cpp
*                     HttpMessage.cs
*                     
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 April 2017
*/


using System;
using System.IO;
using System.Reflection;
using System.Threading;

namespace ConsoleClient
{
    class TestClient2
    {
        private static bool ProcessCommandLine(string[] args)
        {
            Console.WriteLine("\n //----------------------< Remote Client 2 >--------------------//\n");
            Console.WriteLine("   Command line argument:");
            Console.WriteLine("    1 - Local root path where files are stored;");
            Console.WriteLine("    2 - Port number which receive socket is bound to.");
            Console.WriteLine("\n   In this demonstration,");
            Console.WriteLine("   The files to be uploaded are stored in \"TestFiles2\" folder;");
            Console.WriteLine("   The files downloaded from reposiroty are stored in \"Demonstration2\" folder.");
            Console.WriteLine("\n We have two console clients communicate concurrently with remote repository,");
            Console.WriteLine(" they exchange HTTP style messages asychronously.");

            string currentExeDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

            if (args.Length < 2)
            {
                Console.WriteLine("Invalid command line arguments!");
                return false;
            }

            if (!Directory.Exists(currentExeDir + "\\" + args[0]))
            {
                Console.WriteLine("Root path \"{0}\" does not exist!", args[0]);
                return false;
            }
            uint portNum;
            if (!uint.TryParse(args[1], out portNum))
            {
                Console.WriteLine("Invalid port \"{0}\"!", args[1]);
                return false;
            }
            return true;
        }

        private static Thread createMessageHandleThread(myShim shim)
        {
            Thread msgHandlTh = new Thread(
                () =>
                {
                    while (true)
                    {
                        string msgStr = shim.GetMessage();
                        HttpMessageCS msg = HttpMessageCS.parseMessage(msgStr);
                        if (msg.findValue("Command") == "quit")
                            break;

                        Console.WriteLine("\n\n////////////////////////////////////////");
                        Console.WriteLine("// Got reply message from repository: //");
                        Console.WriteLine("////////////////////////////////////////");
                        Console.WriteLine(msg.headerToString());
                    }
                });
            return msgHandlTh;
        }


        private static HttpMessageCS createFileUploadMsg(string category, string fileName, uint localPort)
        {
            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "File Upload");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("File Name", fileName);
            msgCS.addAttribute("Category", category);
            msgCS.addAttribute("Source", "localhost:" + localPort);
            msgCS.addAttribute("Destination", "localhost:8080");
            return msgCS;
        }

        private static void uploadFile(myShim shim, string file, uint portNum)
        {
            HttpMessageCS fileMsg1 = createFileUploadMsg("Category2", Path.GetFullPath(file), portNum);
            string msgStrToSend = fileMsg1.toString();
            Console.WriteLine("\n\n//////////////////////");
            Console.WriteLine("// Sending message: //");
            Console.WriteLine("//////////////////////");

            Console.WriteLine(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }


        private static void demoDownloadFile(myShim shim, string category, string file, uint port)
        {
            Console.WriteLine("\n<----------- Demonstrating downloading file from repository -------------->");
            Console.WriteLine("\n Downloading file \"{0}\"...", file);
            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "File Download");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("File Name", file);
            msgCS.addAttribute("Category", category);
            msgCS.addAttribute("Source", "localhost:" + port);
            msgCS.addAttribute("Destination", "localhost:8080");

            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n\n//////////////////////");
            Console.WriteLine("// Sending message: //");
            Console.WriteLine("//////////////////////");

            Console.WriteLine(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }

        private static void demoUploadFiles(myShim shim, uint portNum)
        {
            string currentExeDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

            Console.WriteLine("\n Uploading file \"Child.h\"...");
            uploadFile(shim, currentExeDir + "\\" + "..\\..\\..\\TestFiles2\\Child.h", portNum);

            Console.WriteLine("\n Uploading file \"Child.cpp\"...");
            uploadFile(shim, currentExeDir + "\\" + "..\\..\\..\\TestFiles2\\Child.cpp", portNum);

            Console.WriteLine("\n Uploading file \"Child2.h\"...");
            uploadFile(shim, currentExeDir + "\\" + "..\\..\\..\\TestFiles2\\Child2.h", portNum);

            Console.WriteLine("\n Uploading file \"Grandparent.h\"...");
            uploadFile(shim, currentExeDir + "\\" + "..\\..\\..\\TestFiles2\\Grandparent.h", portNum);

            Console.WriteLine("\n Uploading file \"Parent.h\"...");
            uploadFile(shim, currentExeDir + "\\" + "..\\..\\..\\TestFiles2\\Parent.h", portNum);

            Console.WriteLine("\n Uploading file \"Parent.h\"...");
            uploadFile(shim, currentExeDir + "\\" + "..\\..\\..\\TestFiles2\\Parent.h", portNum);

            Console.WriteLine("\n Uploading file \"Test.cpp\"...");
            uploadFile(shim, currentExeDir + "\\" + "..\\..\\..\\TestFiles2\\Test.cpp", portNum);
        }


        private static void demoGetAllCategories(myShim shim, uint localPort)
        {
            Console.WriteLine("\n<----------- Demonstrating querying all categories in repository -------------->");

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "Get All Categories");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Source", "localhost:" + localPort);
            msgCS.addAttribute("Destination", "localhost:8080");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n\n//////////////////////");
            Console.WriteLine("// Sending message: //");
            Console.WriteLine("//////////////////////"); Console.WriteLine(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }


        private static void demoGetFilesInCategory(myShim shim, uint localPort)
        {
            Console.WriteLine("\n<----------- Demonstrating querying all files in a category -------------->");

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "Get Files In category");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Category", "Category2");
            msgCS.addAttribute("Source", "localhost:" + localPort);
            msgCS.addAttribute("Destination", "localhost:8080");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n\n//////////////////////");
            Console.WriteLine("// Sending message: //");
            Console.WriteLine("//////////////////////"); Console.WriteLine(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }


        private static void demoPublish(myShim shim, uint localPort)
        {
            Console.WriteLine("\n<----------- Demonstrating building dependencies and publish all files in a category -------------->");

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "Publish");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Category", "Category2");
            msgCS.addAttribute("Demonstration", "ON");
            msgCS.addAttribute("Source", "localhost:" + localPort);
            msgCS.addAttribute("Destination", "localhost:8080");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n\n//////////////////////");
            Console.WriteLine("// Sending message: //");
            Console.WriteLine("//////////////////////"); Console.WriteLine(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }


        private static void demoDeleteFile(myShim shim, string category, string file, uint port)
        {
            Console.WriteLine("\n<----------- Demonstrating deleting file from repository -------------->");

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "Remove File");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Category", category);
            msgCS.addAttribute("File Name", file);
            msgCS.addAttribute("Source", "localhost:" + port);
            msgCS.addAttribute("Destination", "localhost:8080");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n\n//////////////////////");
            Console.WriteLine("// Sending message: //");
            Console.WriteLine("//////////////////////"); Console.WriteLine(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }


        static void Main(string[] args)
        {
            if (!ProcessCommandLine(args))
                return;
            string currentExeDir = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);

            uint portNum = uint.Parse(args[1]);
            myShim shim = new myShim(currentExeDir + "\\" + args[0], portNum, 6);

            if (!shim.start_server())
            {
                Console.WriteLine("\n\n Unable tp start local listener on port");
                return;
            }
            if (!shim.connect_to_server("localhost", 8080))
            {
                Console.WriteLine("\n\n Unable tp connect to remote server");
                return;
            }
            Thread msgHandleTH_ = createMessageHandleThread(shim);
            msgHandleTH_.Start();

            demoUploadFiles(shim, portNum);
            demoGetAllCategories(shim, portNum);
            demoGetFilesInCategory(shim, portNum);
            msgHandleTH_.Join();
        }
    }
}
