<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Default.aspx.cs" Inherits="Microsoft.Samples.ServiceHosting.Thumbnails._Default" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head id="Head1" runat="server">
    <title>Azure Rasterizer</title>
</head>
<body>
    <form id="form1" runat="server">
    <asp:ScriptManager ID="sm1" runat="server" />
    <div>
        Upload input: <asp:FileUpload ID="upload" runat="server" /> <asp:Button ID="submitButton" runat="server" Text="Submit" OnClick="submitButton_Click" />
        <asp:CheckBox id="isHeightMap" Text="Is height map" TextAlign="Right" AutoPostBack="True" runat="server" />
    </div>
    <div>

        <asp:UpdatePanel ID="up1" runat="server">
        <ContentTemplate>
        <asp:ListView ID="results" runat="server">
            <LayoutTemplate>
                <asp:Image ID="itemPlaceholder" runat="server" />
            </LayoutTemplate>
            <ItemTemplate>
                <!--<asp:Image ID="photoImage" runat="server" ImageUrl='<(...)%# Eval("Url") %>' />-->
                <a href='<%# Eval("Url") %>' runat="server"> <%# Eval("Url") %> </a> <br />
            </ItemTemplate>
        </asp:ListView>
                   <asp:Timer ID="timer1" runat="server" Interval="1000" />
            <asp:GridView ID="results2" runat="server" AutoGenerateColumns="False" 
                DataSourceID="ObjectDataSource1">
            </asp:GridView>
            <asp:ObjectDataSource ID="ObjectDataSource1" runat="server" 
                SelectMethod="ListBlobs" 
                TypeName="Microsoft.WindowsAzure.StorageClient.CloudBlobDirectory">
            </asp:ObjectDataSource>
        </ContentTemplate>
    </asp:UpdatePanel>
 
    </div>
    </form>
</body>
</html>
