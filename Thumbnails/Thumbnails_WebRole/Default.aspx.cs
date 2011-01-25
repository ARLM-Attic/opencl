//
// <copyright file="Default.aspx.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//
using System;
using System.Linq;
using System.Net;
using System.Configuration;
using System.Collections;
using Microsoft.WindowsAzure;
using Microsoft.WindowsAzure.StorageClient;
using Microsoft.WindowsAzure.ServiceRuntime;

using RasterizerTables;

namespace Microsoft.Samples.ServiceHosting.Thumbnails
{
    public partial class _Default : System.Web.UI.Page
    {
        private static CloudBlobClient blobStorage;
        private static CloudQueueClient queueStorage;
        private static TableServiceContext tableServiceContext;

        private static bool s_createdContainerAndQueue = false;
        private static object s_lock = new Object();

        private void CreateOnceContainerAndQueue()
        {
            if (s_createdContainerAndQueue)
                return;
            lock (s_lock)
            {
                if (s_createdContainerAndQueue)
                {
                    return;
                }

                try
                {
                    var storageAccount = CloudStorageAccount.Parse(RoleEnvironment.GetConfigurationSettingValue("DataConnectionString"));
                    //var storageAccount = CloudStorageAccount.FromConfigurationSetting("DataConnectionString");

                    blobStorage = storageAccount.CreateCloudBlobClient();
                    CloudBlobContainer container = blobStorage.GetContainerReference("datasets");

                    container.CreateIfNotExist();

                    var permissions = container.GetPermissions();

                    permissions.PublicAccess = BlobContainerPublicAccessType.Container;

                    container.SetPermissions(permissions);

                    queueStorage = storageAccount.CreateCloudQueueClient();
                    CloudQueue queue = queueStorage.GetQueueReference("inputreceiver");

                    queue.CreateIfNotExist();

                    tableServiceContext = new TableServiceContext(storageAccount.TableEndpoint.ToString(), storageAccount.Credentials);
                }
                catch (WebException)
                {
                    // display a nice error message if the local development storage tool is not running or if there is 
                    // an error in the account configuration that causes this exception
                    throw new WebException("The Windows Azure storage services cannot be contacted " +
                         "via the current account configuration or the local development storage tool is not running. " +
                         "Please start the development storage tool if you run the service locally!");
                }

                s_createdContainerAndQueue = true;
            }
        }

        private CloudBlobContainer GetResultsContainer()
        {
            CreateOnceContainerAndQueue();
            return blobStorage.GetContainerReference("datasets");
        }

        private CloudQueue GetInputReceiverQueue()
        {
            CreateOnceContainerAndQueue();
            return queueStorage.GetQueueReference("inputreceiver");
        }

        private TableServiceContext GetTableServiceContext()
        {
            CreateOnceContainerAndQueue();
            return tableServiceContext;
        }

        private string GetMimeType(string Filename)
        {
            try
            {
                string ext = System.IO.Path.GetExtension(Filename).ToLowerInvariant();
                Microsoft.Win32.RegistryKey key = Microsoft.Win32.Registry.ClassesRoot.OpenSubKey(ext);
                if (key != null)
                {
                    string contentType = key.GetValue("Content Type") as String;
                    if (!String.IsNullOrEmpty(contentType))
                    {
                        return contentType;
                    }
                }
            }
            catch 
            {
            }
            return "application/octet-stream";
        }

        protected void submitButton_Click(object sender, EventArgs e)
        {
            if (upload.HasFile)
            {
                var ext  =  System.IO.Path.GetExtension(upload.FileName);
   
                var name = string.Format("{0}_{1}.txt", upload.FileName, Guid.NewGuid());

                var blob = GetResultsContainer().GetBlockBlobReference(name);
                blob.Properties.ContentType = GetMimeType(upload.FileName);
                blob.UploadFromStream(upload.FileContent);

                if (isHeightMap.Checked)
                    name += "\ny";
                else
                    name += "\nn";

                name += "\n" + upload.FileName;

                if (runParallel.Checked)
                    name += "\ny";
                else
                    name += "\nn";

                GetInputReceiverQueue().AddMessage(new CloudQueueMessage(System.Text.Encoding.UTF8.GetBytes(name)));

                System.Diagnostics.Trace.WriteLine(String.Format("Enqueued '{0}'", name));
            }
        }

        protected void Page_PreRender(object sender, EventArgs e)
        {
            try
            {
                // Get all elements from ResultsTable table
                var qResults = from c in GetTableServiceContext().CreateQuery<ResultsTable>("ResultsTable").Execute() select c;

                results.DataSource = from o in qResults
                                     select
                                     new
                                     {
                                         DatasetUrl = o.DatasetURL,
                                         ResultUrl = o.ResultURL,
                                         Filename = o.DatasetFilename,
                                         Type = o.IsParallel ? "Parallel" : "Sequential",
                                         Time = o.Time
                                     };

                results.DataBind();
            }
            catch (Exception)
            {
            }
        }
    }
}
