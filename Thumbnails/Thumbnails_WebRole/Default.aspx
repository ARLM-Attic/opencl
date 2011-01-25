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
        Upload input: <asp:FileUpload ID="upload" runat="server" />
        <asp:Button ID="submitButton" runat="server" Text="Submit" OnClick="submitButton_Click" />
        <asp:CheckBox id="isHeightMap" Text="Is height map" TextAlign="Right" AutoPostBack="True" runat="server" />
        <asp:CheckBox id="runParallel" Text="Run in parallel" TextAlign="Right" AutoPostBack="True" runat="server" Checked="True"/>
    </div>
    <div>

    <asp:UpdatePanel ID="up1" runat="server">
        <ContentTemplate>
        <asp:ListView ID="results" runat="server">
            <LayoutTemplate>
                <asp:Image ID="itemPlaceholder" runat="server" />
            </LayoutTemplate>
            <ItemTemplate>
                <%# Eval("Filename") %> - 
                <a id="A1" href='<%# Eval("DatasetUrl") %>' runat="server"> Input </a> - 
                <a id="A2" href='<%# Eval("ResultUrl") %>' runat="server"> Result </a> - 
                <%# Eval("Time") %>ms - 
                <%# Eval("Type") %> 
                <br />
            </ItemTemplate>
        </asp:ListView>
        <br />
            <asp:Timer ID="timer1" runat="server" Interval="1000" />
        </ContentTemplate>
    </asp:UpdatePanel>
 
    </div>
    </form>
</body>
</html>
