/////////////////////////////////////////////////////////////////////////////
//  MainWindow.xaml.cs - Client side GUI                                   //
//  Language:     C#, VS 2015                                              //
//  Platform:     SurfaceBook, Windows 10 Pro                              //
//  Application:  Project1 for CSE687 - Object Oriented Design             //
//  Author:       Weijun Cai                                               //
/////////////////////////////////////////////////////////////////////////////
/*
*   Module Operations
*   -----------------
*   This module contains the implementaion of client GUI control logic
*   communicate with repository server using socket-based HTTP style messaging
*   
*/
/*
*   Build Process
*   -------------
*   - Required files: CLIshim.h, CLIshim.cpp
*                     HttpMessageCS.cs
*                     MainWindow.xaml.cs
*                     
*   Maintenance History
*   -------------------
*   ver 1.0 : 28 April 2017
*/

using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Windows;
using System.Windows.Controls;

namespace ClientGUI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private myShim shim;
        private string uploadFilePath;
        private string localSavePath;
        private string rootPath = "..\\..\\..\\LocalStorage";
        private uint remotePort = 8080;
        private Dictionary<string, List<Button>> downloadBtnCollec;
        private Dictionary<Button, Tuple<string, string>> btnFileCategoryInfo;
        private Dictionary<string, Button> viewBtnCollec;
        Thread msgHandleThrd = null;

        public MainWindow()
        {
            InitializeComponent();
        }




        // registered handler, called on window load to initialize
        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            shim = new myShim(rootPath, 9098, 6);
            if (!shim.start_server())
            {
                MessageBox.Show("Unable tp start local listener", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                Close();
            }
            disconnectBtn.IsEnabled = false;
            downloadBtnCollec = new Dictionary<string, List<Button>>();
            btnFileCategoryInfo = new Dictionary<Button, Tuple<string, string>>();
            viewBtnCollec = new Dictionary<string, Button>();
            // the child thread to handle repo messages
            msgHandleThrd = new Thread(
                () =>
                {
                    while (true)
                    {
                        HttpMessageCS serverMsg = HttpMessageCS.parseMessage(shim.GetMessage());
                        if (serverMsg.findValue("Command") == "quit")
                            break;

                        Console.WriteLine("\n\n Got message from server:\n");
                        Console.WriteLine(serverMsg.headerToString());

                        Action<HttpMessageCS> action = new Action<HttpMessageCS>(handleMessage);
                        object[] args = { serverMsg };
                        Dispatcher.Invoke(action, args);

                    }
                });
            msgHandleThrd.Start();
        }


        private void Window_Unloaded(object sender, RoutedEventArgs e)
        {

        }


        // handle all kinds of repo responses
        private void handleMessage(HttpMessageCS msg)
        {
            string serverMsg = msg.toString();
            txtListBox_repoMsg.Items.Add(serverMsg);
            string command = msg.findValue("Command");
            if (command.Length == 0)
            {
                Console.WriteLine("\nInvalid message!");
                return;
            }
            ////////////////////////////////////////////////
            // server send the name of all categories to client
            // - for each category, create a new treeItem
            if (command == "Set All Categories")
            {
                handleSetAllCategoriesReply(msg);
                return;
            }
            if (command == "Set Files In Category")
            {
                handleSetFilesInCategoryReply(msg);
                return;
            }
            // dependency analyze complete, can now download html file and view in browser
            if (command == "AnalyzeAck")
            {
                handleAnalyzeAck(msg);
                return;
            }
            if (command == "FileAck")
            {
                handleFileAck(msg);
                return;
            }
            /////////////////////////////////////////////////////////
            // server uploaded a file to client at client's request
            // - at this time, file has already been accepted by underlying receiver
            //   and stored in a category folder at root directory successfully
            // - pop a message box to notify
            // -- if downloaded from "file" tab, no special actions
            // -- if downloaded from "category" tab, update category treeItem, enable view button
            if (command == "File Upload")
            {
                handleFileUpload(msg);
                return;
            }
        }

        // got the name of all categories from repo
        // update treeview to show all categories
        private void handleSetAllCategoriesReply(HttpMessageCS msg)
        {
            var categories = msg.getAllValuesOfAttribute("Category");
            if (categories.Count == 0)
                return;
            foreach (var categoryName in categories)
            {
                StackPanel sp = new StackPanel();
                sp.Name = "stack_" + categoryName;
                sp.Orientation = Orientation.Horizontal;

                TreeViewItem categoryTreeItem = new TreeViewItem();
                categoryTreeItem.Name = "treeItem_" + categoryName;
                categoryTreeItem.Header = categoryName;
                sp.Children.Add(categoryTreeItem);

                Button btn_analyze = new Button();
                btn_analyze.Name = "btn_analyzePub_" + categoryName;
                btn_analyze.Content = "Analyze";
                btn_analyze.Click += btn_analyzePub_Click;
                btn_analyze.IsEnabled = false;
                sp.Children.Add(btn_analyze);

                downloadBtnCollec.Add(categoryName, new List<Button>());

                tree_categories.Items.Add(sp);
                categoryTreeItem.IsExpanded = true;
            }
        }


        // got the name of all files in a category
        // populate the correpsonding category tree node with controls of all files
        private void handleSetFilesInCategoryReply(HttpMessageCS msg)
        {
            var fileNames = msg.getAllValuesOfAttribute("File Name");
            string categoryName = msg.findValue("Category");
            string categoryItemName = "treeItem_" + categoryName;
            TreeViewItem categoryTreeItem = (TreeViewItem)LogicalTreeHelper.FindLogicalNode(this, categoryItemName);
            Button analyBtn = (Button)LogicalTreeHelper.FindLogicalNode(this, "btn_analyzePub_" + categoryName);
            analyBtn.IsEnabled = true;
            if (categoryTreeItem == null)
            {
                Console.WriteLine("\nCannot find category \"{0}\" to update files.", categoryItemName);
                return;
            }
            foreach (var fileName in fileNames)
            {
                DockPanel fileControlSet = createControlsForFile(categoryName, fileName);
                categoryTreeItem.Items.Add(fileControlSet);
            }
            categoryTreeItem.IsExpanded = true;
        }

        // got requested file from repo, perfrom different actions based on the intended operation
        private void handleFileUpload(HttpMessageCS msg)
        {
            if (msg.findValue("GUI Operation") == "Enable View HTML")
            {
                string category = msg.findValue("Category");
                string fileName = System.IO.Path.GetFileNameWithoutExtension(msg.findValue("File Name"));
                string viewBtnName = category + fileName;
 
                viewBtnCollec[viewBtnName].IsEnabled = true;
            }
        }

        // got acknowladgement about file transmission from server, display it
        private void handleFileAck(HttpMessageCS msg)
        {
            string additionalMsg = msg.findValue("Additional Message");
            MessageBox.Show(additionalMsg, "Message from server", MessageBoxButton.OK, MessageBoxImage.Information);
        }

        // got acknowledgement about the completion of file dependency analysis
        private void handleAnalyzeAck(HttpMessageCS msg)
        {
            string categoryName = msg.findValue("Category");
            foreach (var btn in downloadBtnCollec[categoryName])
                btn.IsEnabled = true;
            Button analyze_btn = (Button)LogicalTreeHelper.FindLogicalNode(this, "btn_analyzePub_" + categoryName);
            analyze_btn.IsEnabled = false;
        }

        // create contols for a newly added file
        private DockPanel createControlsForFile(string categoryName, string fileName)
        {
            Tuple<string, string> file_cate_info = new Tuple<string, string>(categoryName, fileName);

            DockPanel dp = new DockPanel();
            string categoryItemName = "treeItem_" + categoryName;

            Label labl_fileName = new Label();
            labl_fileName.Content = fileName;
            DockPanel.SetDock(labl_fileName, Dock.Left);
            dp.Children.Add(labl_fileName);

            Button btn_viewHtml = new Button();
            btn_viewHtml.Content = "View in browser";
            btn_viewHtml.IsEnabled = false;
            DockPanel.SetDock(btn_viewHtml, Dock.Right);
            btn_viewHtml.Click += btn_viewHtml_Click;
            dp.Children.Add(btn_viewHtml);
            btnFileCategoryInfo.Add(btn_viewHtml, file_cate_info);
            viewBtnCollec.Add(categoryName + fileName, btn_viewHtml);

            Button btn_remvFileFromRepo = new Button();
            btn_remvFileFromRepo.Content = "Delete from Repository";
            DockPanel.SetDock(btn_remvFileFromRepo, Dock.Right);
            btn_remvFileFromRepo.Click += btn_remvFileFromRepo_Click;
            dp.Children.Add(btn_remvFileFromRepo);
            btnFileCategoryInfo.Add(btn_remvFileFromRepo, file_cate_info);

            Button btn_downloadHTML = new Button();
            btn_downloadHTML.Content = "Download";
            btn_downloadHTML.IsEnabled = false;
            DockPanel.SetDock(btn_downloadHTML, Dock.Right);
            btn_downloadHTML.Click += btn_downloadHTML_Click;
            dp.Children.Add(btn_downloadHTML);
            btnFileCategoryInfo.Add(btn_downloadHTML, file_cate_info);
            downloadBtnCollec[categoryName].Add(btn_downloadHTML);
            return dp;
        }




        private bool isLegalFileCategoryName(string category)
        {
            if (string.IsNullOrWhiteSpace(category))
                return false;
            HashSet<char> illegalChars = new HashSet<char> { '/', '\\', ':', '*', '?', '"', '<', '>', '|' };
            foreach (char c in category)
            {
                if (illegalChars.Contains(c))
                    return false;
            }
            return true;
        }


        private void connectBtn_Click(object sender, RoutedEventArgs e)
        {
            
            if(txtBox_portNum.Text.Length == 0)
            {
                MessageBox.Show("Invalid port number", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            uint port;

            if(!uint.TryParse(txtBox_portNum.Text, out port))
            {
                MessageBox.Show("Invalid port number", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            if (!shim.connect_to_server("localhost", port))
            {
                MessageBox.Show("Unable tp start local listener", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                Console.WriteLine("\n\n Unable tp connect to remote server");
                Close();
            }
            connectBtn.IsEnabled = false;
            disconnectBtn.IsEnabled = true;
        }

        private void btn_getUploadPath_Click(object sender, RoutedEventArgs e)
        {
            OpenFileDialog openDlg = new OpenFileDialog();
            openDlg.Filter = "C++ Source Files |*.h;*.cpp";
            string path = "..\\..\\..\\TestCase";
            //string path = System.IO.Path.Combine(System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location), "..\\..\\..\\ToSend");
            string fullpath = System.IO.Path.GetFullPath(path);
            openDlg.InitialDirectory = fullpath;

            if (openDlg.ShowDialog() == true)
            {
                txt_uploadPath.Text = openDlg.FileName;
                uploadFilePath = System.IO.Path.GetDirectoryName(openDlg.FileName);
            }
        }

        private void btn_fileUpload_Click(object sender, RoutedEventArgs e)
        {
            if (!System.IO.File.Exists(txt_uploadPath.Text))
            {
                MessageBox.Show("Local upload file does not exist", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!isLegalFileCategoryName(fileUploadCategory.Text))
            {
                MessageBox.Show("Invalid category name", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }
            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "File Upload");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("File Name", txt_uploadPath.Text);
            msgCS.addAttribute("Category", fileUploadCategory.Text);
            msgCS.addAttribute("Destination", "localhost:" + remotePort);
            msgCS.addAttribute("Source", "localhost:9098");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n Sending message:\n");
            Console.WriteLine(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }

        private void btn_fileDownload_Click(object sender, RoutedEventArgs e)
        {
            if (!System.IO.Directory.Exists(txt_localSavePath.Text))
            {
                MessageBox.Show("Local destination path does not exist", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!isLegalFileCategoryName(fileDownloadCategory.Text))
            {
                MessageBox.Show("Invalid category name", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            if (!isLegalFileCategoryName(fileDownloadName.Text))
            {
                MessageBox.Show("Invalid file name", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "File Download");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("File Name", fileDownloadName.Text);
            msgCS.addAttribute("Category", fileDownloadCategory.Text);
            msgCS.addAttribute("Local Save Path", txt_localSavePath.Text);
            msgCS.addAttribute("Destination", "localhost:" + remotePort);
            msgCS.addAttribute("Source", "localhost:9098");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n Sending message:\n");
            Console.WriteLine(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }


        // query all category (folder) names in reposiroty
        // server reply used to update tree view
        // by message handler to dynamically generate control
        // no files are downloaded at this request
        private void btn_getAllCategories_Click(object sender, RoutedEventArgs e)
        {
            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "Get All Categories");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Destination", "localhost:" + remotePort);
            msgCS.addAttribute("Source", "localhost:9098");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n Sending message:\n");
            Console.WriteLine(msgStrToSend);
            txtListBox_clientMsg.Items.Add(msgStrToSend);
            shim.PostMessage(msgStrToSend);
        }


        // query the names of all files in a specified folder in repository
        // server reply used to update the tree view of its corresponding category
        // by message handler to dynamically generate control
        // no files are downloaded at this request
        private void btn_getFileInCategory_Click(object sender, RoutedEventArgs e)
        {
            if (txt_categoryToGetFiles.Text.Length == 0)
            {
                MessageBox.Show("Invalid category name", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "Get Files In category");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Category", txt_categoryToGetFiles.Text);
            msgCS.addAttribute("Destination", "localhost:" + remotePort);
            msgCS.addAttribute("Source", "localhost:9098");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n Sending message:\n");
            Console.WriteLine(msgStrToSend);
            txtListBox_clientMsg.Items.Add(msgStrToSend);

            shim.PostMessage(msgStrToSend);
        }

        // must download before viewing
        private void btn_viewHtml_Click(object sender, RoutedEventArgs e)
        {
            Button btn = (Button)sender;
            var cate_file_name = btnFileCategoryInfo[btn];

            Process p = new Process();
            string viewCommand = "start \"\" \"" + System.IO.Path.Combine(System.IO.Path.GetFullPath(rootPath), cate_file_name.Item1, cate_file_name.Item2) + ".htm\"";
            System.IO.File.WriteAllText("view.bat", viewCommand);

            p.StartInfo.FileName = "view.bat";
            p.Start();
        }

        private void btn_remvFileFromRepo_Click(object sender, RoutedEventArgs e)
        {
            Button btn = (Button)sender;
            var cate_file_name = btnFileCategoryInfo[btn];
            btn.IsEnabled = false;

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "Remove File");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Category", cate_file_name.Item1);
            msgCS.addAttribute("File Name", cate_file_name.Item2);
            msgCS.addAttribute("Destination", "localhost:" + remotePort);
            msgCS.addAttribute("Source", "localhost:9098");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n Sending message:\n");
            Console.WriteLine(msgStrToSend);
            txtListBox_clientMsg.Items.Add(msgStrToSend);

            shim.PostMessage(msgStrToSend);
        }

        private void btn_downloadHTML_Click(object sender, RoutedEventArgs e)
        {
            Button btn = (Button)sender;
            var cate_file_name = btnFileCategoryInfo[btn];

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "File Download");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("File Name", cate_file_name.Item2 + ".htm");
            msgCS.addAttribute("Category", cate_file_name.Item1);
            msgCS.addAttribute("GUI Operation", "Enable View HTML");
            msgCS.addAttribute("Destination", "localhost:" + remotePort);
            msgCS.addAttribute("Source", "localhost:9098");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n Sending message:\n");
            Console.WriteLine(msgStrToSend);
            txtListBox_clientMsg.Items.Add(msgStrToSend);

            shim.PostMessage(msgStrToSend);
        }

        private void btn_analyzePub_Click(object sender, RoutedEventArgs e)
        {
            Button btn = (Button)sender;
            string prefix = "btn_analyzePub_";
            string categoryName = btn.Name.Substring(prefix.Length);

            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "Publish");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Category", categoryName);
            msgCS.addAttribute("GUI Operation", "Publish");
            msgCS.addAttribute("Demonstration", "OFF");
            msgCS.addAttribute("Destination", "localhost:" + remotePort);
            msgCS.addAttribute("Source", "localhost:9098");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n Sending message:\n");
            Console.WriteLine(msgStrToSend);
            txtListBox_clientMsg.Items.Add(msgStrToSend);

            shim.PostMessage(msgStrToSend);
        }

        private void disconnectBtn_Click(object sender, RoutedEventArgs e)
        {
            HttpMessageCS msgCS = new HttpMessageCS();
            msgCS.addAttribute("POST", "Message");
            msgCS.addAttribute("Command", "quit");
            msgCS.addAttribute("Mode", "Oneway");
            msgCS.addAttribute("Destination", "localhost:" + remotePort);
            msgCS.addAttribute("Source", "localhost:9098");
            string msgStrToSend = msgCS.toString();
            Console.WriteLine("\n Sending message:\n");
            Console.WriteLine(msgStrToSend);
            txtListBox_clientMsg.Items.Add(msgStrToSend);

            shim.PostMessage(msgStrToSend);
            ((Button)sender).IsEnabled = false;
        }

        private void setRootPathBtn_Click(object sender, RoutedEventArgs e)
        {
            if (!System.IO.Directory.Exists(userSetRootPath.Text))
            {
                MessageBox.Show("Local destination path does not exist", "Error", MessageBoxButton.OK, MessageBoxImage.Warning);
                return;
            }

            rootPath = userSetRootPath.Text;
        }
    }
}
