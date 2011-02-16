using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Data.Services.Client;
using Microsoft.WindowsAzure.StorageClient;

namespace RasterizerTables
{
    public class ResultsTable : TableServiceEntity
    {
        public ResultsTable(string partitionKey, string rowKey) : base(partitionKey, rowKey) { }

        // Rows need a unique partition key – so create a new guid for every row
        public ResultsTable() : this(Guid.NewGuid().ToString(), String.Empty) { }

        // My table row
        public Uri ResultURL { get; set; }
        public Uri DatasetURL { get; set; }
        public string DatasetFilename { get; set; }
        public double Time { get; set; }
        public bool IsHeightMap { get; set; }
        public bool IsParallel { get; set; }
    }
}
