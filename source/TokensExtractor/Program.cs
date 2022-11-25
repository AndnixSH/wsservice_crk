using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using System.Windows.Forms;

using System.IO;
using System.Text;
using System.Xml;

using System.Security.Cryptography;
using System.Security.Cryptography.Xml;

using System.Runtime.InteropServices;


namespace TokensExtractor
{
    static class Program
    {
        public static IntPtr NativeUtf8FromString(string managedString)
        {
            int len = Encoding.UTF8.GetByteCount(managedString);
            byte[] buffer = new byte[len + 1];
            Encoding.UTF8.GetBytes(managedString, 0, managedString.Length, buffer, 0);
            IntPtr nativeUtf8 = Marshal.AllocHGlobal(buffer.Length);
            Marshal.Copy(buffer, 0, nativeUtf8, buffer.Length);
            return nativeUtf8;
        }

        public static string StringFromNativeUtf8(IntPtr nativeUtf8)
        {
            int len = 0;
            while (Marshal.ReadByte(nativeUtf8, len) != 0) ++len;
            if (len == 0) return string.Empty;
            byte[] buffer = new byte[len];
            Marshal.Copy(nativeUtf8, buffer, 0, buffer.Length);
            return Encoding.UTF8.GetString(buffer);
        }

        public static byte[] String2Bytes(string str)
        {
            return Encoding.ASCII.GetBytes(str);
        }
        public static string Bytes2String(byte[] b, int pos, int size)
        {
            return Encoding.ASCII.GetString(b,pos,size);
        }

        public static int BinIndexOf(byte[] arrayToSearchThrough, byte[] patternToFind, int StartingPos)
        {
            if (patternToFind.Length > arrayToSearchThrough.Length)
                return -1;
            for (int i = StartingPos; i < arrayToSearchThrough.Length - patternToFind.Length; i++)
            {
                bool found = true;
                for (int j = 0; j < patternToFind.Length; j++)
                {
                    if (arrayToSearchThrough[i + j] != patternToFind[j])
                    {
                        found = false;
                        break;
                    }
                }
                if (found)
                {
                    return i;
                }
            }
            return -1;
        }

        public static List<XmlDocument> tokens_LIC;


        [DllImport("wsclient.dll")]
        static extern UInt32 WSLicenseOpen(out IntPtr Context);
        [DllImport("wsclient.dll")]
        static extern UInt32 WSLicenseClose(IntPtr Context);
        [DllImport("wsclient.dll")]
        static extern UInt32 WSLicenseInstallLicense(IntPtr Context, UInt32 Size, string LicenseXml, IntPtr Unk);
        [DllImport("wsclient.dll")]
        static extern UInt32 WSLicenseUninstallLicense(IntPtr Context, IntPtr Unk, ref Guid ProductID);
        [DllImport("wsclient.dll")]
        static extern UInt32 WSLicenseGetLicensesForProducts(IntPtr Context, UInt32 GuidCount, IntPtr Guids, out IntPtr LicPtrArray);
        [DllImport("wsclient.dll")]
        static extern UInt32 WSLicenseGetAllValidAppCategoryIds(IntPtr Context, out UInt32 PidCount, out IntPtr GuidArray);
        [DllImport("kernel32.dll")]
        static extern UInt32 LocalFree(IntPtr hMem);

        static public bool ReadTokens(bool bBruteForce = false)
        {
            try
            {
                tokens_LIC = new List<XmlDocument>();

                if (bBruteForce)
                {
                    // dig tokens.dat directly 

                    byte[] Tokens;
                    string TokensFileName;

                    TokensFileName = Environment.GetEnvironmentVariable("SystemRoot") +
                        "\\ServiceProfiles\\LocalService\\AppData\\Local\\Microsoft\\WSLicense\\tokens.dat";
                    Tokens = File.ReadAllBytes(TokensFileName);

                    byte[] LicenseStartPattern = String2Bytes("<License");
                    byte[] LicenseEndPattern = String2Bytes("</License>");


                    for (int pos = 0, pos_end; ; )
                    {
                        pos = BinIndexOf(Tokens, LicenseStartPattern, pos);
                        if (pos < 0) break;
                        pos_end = BinIndexOf(Tokens, LicenseEndPattern, pos + 1);
                        if (pos_end < 0) break;
                        pos_end += LicenseEndPattern.Length;

                        XmlDocument x = new XmlDocument();
                        x.PreserveWhitespace = true;

                        try
                        {
                            x.InnerXml = Bytes2String(Tokens, pos, pos_end - pos);
                            tokens_LIC.Add(x);
                        }
                        catch (Exception)
                        {
                            // xml content was bad. we have mined remnants of deleted license
                            // need to skip this shit
                            pos_end = pos + 1;
                        }

                        pos = pos_end;
                    }
                }
                else
                {
                    UInt32 hRes;
                    IntPtr Context;
                    UInt32 PidCount;

                    // normal API search
                    hRes = WSLicenseOpen(out Context);
                    if (hRes != 0)
                        throw new Exception("WSLicenseOpen hr=" + hRes.ToString("X8"));
                    try
                    {
                        IntPtr raw_GuidArray;
                        hRes = WSLicenseGetAllValidAppCategoryIds(Context, out PidCount, out raw_GuidArray);
                        if (hRes != 0)
                            throw new Exception("WSLicenseGetAllValidAppCategoryIds hr=" + hRes.ToString("X8"));
                        try
                        {
                            for(UInt32 i=0;i<PidCount;i++)
                            {
                                IntPtr raw_LicPtrArray;
                                hRes = WSLicenseGetLicensesForProducts(Context, 1, (IntPtr)(raw_GuidArray.ToInt64()+16*i), out raw_LicPtrArray);
                                if (hRes != 0)
                                    throw new Exception("WSLicenseGetLicensesForProducts hr=" + hRes.ToString("X8"));
                                try
                                {
                                    IntPtr raw_sLicense;
                                    raw_sLicense = (IntPtr)Marshal.PtrToStructure(raw_LicPtrArray, typeof(IntPtr));
                                    string sLicense = StringFromNativeUtf8(raw_sLicense);
                                    LocalFree(raw_sLicense);

                                    string sLicenseStripped;
                                    int pos = sLicense.IndexOf("<License");
                                    sLicenseStripped = pos>0 ? sLicense.Substring(pos) : sLicense;

                                    try
                                    {
                                        XmlDocument x = new XmlDocument();
                                        x.PreserveWhitespace = true;
                                        x.InnerXml = sLicenseStripped;
                                        tokens_LIC.Add(x);
                                    }
                                    catch (Exception ex)
                                    {
                                        MessageBox.Show("Invalid XML returned by WSClient API. Error : " + ex.Message + "\n" + sLicense);
                                    }
                                }
                                finally
                                {
                                    LocalFree(raw_LicPtrArray);
                                }
                            }

                        }
                        finally
                        {
                            LocalFree(raw_GuidArray);
                        }
                    }
                    finally
                    {
                        WSLicenseClose(Context);
                    }

                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("ReadTokens "+ex.Message);
                return false;
            }
            return true;
        }

        static Random random;

        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            CryptoConfig.AddAlgorithm(typeof(Security.Cryptography.RSAPKCS1SHA256SignatureDescription), "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256");

            random = new Random();

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            if (ReadTokens() || ReadTokens(true))
                Application.Run(new Form1());
        }

        public static string FormatXml(XmlDocument xd)
        {
            //will hold formatted xml
            StringBuilder sb = new StringBuilder();

            //pumps the formatted xml into the StringBuilder above
            StringWriter sw = new StringWriter(sb);

            //does the formatting
            XmlTextWriter xtw = null;

            try
            {
                //point the xtw at the StringWriter
                xtw = new XmlTextWriter(sw);

                //we want the output formatted
                xtw.Formatting = Formatting.Indented;

                //get the dom to dump its contents into the xtw 
                xd.WriteTo(xtw);
            }
            finally
            {
                //clean up even if error
                if (xtw != null)
                    xtw.Close();
            }

            return sb.ToString();
        }

        public static void SignXml(XmlDocument xmlDoc, RSA key)
        {
            // Check arguments. 
            if (xmlDoc == null)
                throw new ArgumentException("xmlDoc");
            if (key == null)
                throw new ArgumentException("key");


            // Create a SignedXml object.
            SignedXml signedXml = new SignedXml(xmlDoc);

            signedXml.SigningKey = key;

            signedXml.SignedInfo.CanonicalizationMethod = "http://www.w3.org/2001/10/xml-exc-c14n#";
            signedXml.SignedInfo.SignatureMethod = @"http://www.w3.org/2001/04/xmldsig-more#rsa-sha256";

            // Add the reference to the SignedXml object.
            // Create a reference to be signed.
            Reference reference = new Reference();
            reference.DigestMethod = @"http://www.w3.org/2001/04/xmlenc#sha256";
            reference.Uri = "";

            // Add an enveloped transformation to the reference.
            XmlDsigEnvelopedSignatureTransform env = new XmlDsigEnvelopedSignatureTransform();
            reference.AddTransform(env);

            signedXml.AddReference(reference);

            signedXml.ComputeSignature();

            // Get the XML representation of the signature and save 
            // it to an XmlElement object.
            XmlElement xmlDigitalSignature = signedXml.GetXml();

            // Append the element to the XML document.
            xmlDoc.DocumentElement.AppendChild(xmlDoc.ImportNode(xmlDigitalSignature, true));

        }

        public static void SignXmlWithOurKey(XmlDocument xmlDoc)
        {
            CspParameters cspParams = new CspParameters(24);
            cspParams.KeyContainerName = "SignXmlBogusKey";
            RSACryptoServiceProvider key = new RSACryptoServiceProvider(2048, cspParams);

            SignXml(xmlDoc, key);
        }

        public static int RandomNumber(int min, int max)
        {
            return random.Next(min, max+1);
        }

        public static XmlDocument ConvertLicToOEM(XmlDocument Lic)
        {
            try
            {
                XmlNode nodeRoot = Lic.ChildNodes[0];
                if (nodeRoot == null)
                    throw new Exception("XML format error");
                XmlNode nodeBinding = nodeRoot["Binding"];
                if (nodeBinding == null)
                    throw new Exception("<Binding> node not found");
                XmlNode nodePFM = nodeBinding["PFM"];
                if (nodePFM == null)
                    throw new Exception("<PFM> node not found");
                XmlNode nodeProductID = nodeBinding["ProductID"];
                if (nodeProductID == null)
                    throw new Exception("<ProductID> node not found");

                string ProductID = nodeProductID.InnerText;
                string PFM = nodePFM.InnerText;

                XmlDocument LicOem = new XmlDocument();

                LicOem = new XmlDocument();
                LicOem.InnerXml = "<License Version=\"1\" Source=\"OEM\" xmlns=\"urn:schemas-microsoft-com:windows:store:licensing:ls\"><Binding Binding_Type=\"Machine\"><ProductID></ProductID><PFM></PFM></Binding><LicenseInfo Type=\"Full\"><IssuedDate></IssuedDate><LastUpdateDate></LastUpdateDate></LicenseInfo></License>";

                XmlNode xRoot = LicOem.ChildNodes[0];
                XmlNode xBinding = xRoot["Binding"];
                XmlNode xLicenseInfo = xRoot["LicenseInfo"];

                XmlNode xx = xLicenseInfo["ProductID"];
                xBinding["ProductID"].InnerText = ProductID;
                xBinding["PFM"].InnerText = PFM;

                string RandomDate;

                //<IssuedDate>2022-11-01T14:25:14Z</IssuedDate>
                RandomDate = String.Format("{0,4}-{1,2}-{2,2}T{3,2}:{4,2}:{5,2}Z",
                    (Program.RandomNumber(0, 1) + 2011).ToString("D4"),
                    Program.RandomNumber(1, 12).ToString("D2"),
                    Program.RandomNumber(1, 28).ToString("D2"),
                    Program.RandomNumber(0, 23).ToString("D2"),
                    Program.RandomNumber(0, 59).ToString("D2"),
                    Program.RandomNumber(0, 59).ToString("D2")
                );

                xLicenseInfo["IssuedDate"].InnerText = RandomDate;
                xLicenseInfo["LastUpdateDate"].InnerText = RandomDate;

                SignXmlWithOurKey(LicOem);

                return LicOem;
            }
            catch (Exception ex)
            {
                MessageBox.Show("OEM convert " + ex.Message);
                return null;
            }
        }


        public static bool ProductIDFromLic(XmlDocument Lic,ref Guid g)
        {
            try
            {
                string ProductID = Lic.FirstChild["Binding"]["ProductID"].InnerText;
                g = Guid.Parse(ProductID);
                return true;
            }
            catch (Exception)
            {
                return false;
            }
        }

        public static bool InstLic(XmlDocument Lic,bool bTest)
        {
            UInt32 hRes;
            IntPtr Context = new IntPtr();

            try
            {
                hRes = WSLicenseOpen(out Context);
                if (hRes != 0)
                    throw new Exception("WSLicenseOpen hr=" + hRes.ToString("X8"));

                Guid g = new Guid();
                bool bGuidparsed=false;
                try
                {
                    bGuidparsed = ProductIDFromLic(Lic,ref g);
                    hRes = WSLicenseUninstallLicense(Context, (IntPtr)0, ref g);
                }
                catch (Exception)
                {
                }

                hRes = WSLicenseInstallLicense(Context, (UInt32)Lic.InnerXml.Length, Lic.InnerXml, (IntPtr)0);
                if (hRes != 0)
                    throw new Exception("WSLicenseInstallLicense hr=" + hRes.ToString("X8"));

                if (bTest && bGuidparsed)
                {
                    hRes = WSLicenseUninstallLicense(Context, (IntPtr)0, ref g);
                    if (hRes != 0)
                        throw new Exception("WSLicenseUninstallLicense hr=" + hRes.ToString("X8"));
                }

                hRes = WSLicenseClose(Context);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Install license : " + ex.Message);
                if (Context != (IntPtr)0)
                    hRes = WSLicenseClose(Context);
                return false;
            }
            return true;
        }
        public static bool InstLic(string sLic,bool bTest)
        {
            XmlDocument Lic = new XmlDocument();
            try
            {
                Lic.PreserveWhitespace = true;
                Lic.InnerXml = sLic;
            }
            catch (Exception ex)
            {
                MessageBox.Show("Install license : " + ex.Message);
                return false;
            }
            return InstLic(Lic,bTest);
        }


        public static bool UninstLic(XmlDocument Lic)
        {
            UInt32 hRes;
            IntPtr Context = new IntPtr();

            try
            {
                hRes = WSLicenseOpen(out Context);
                if (hRes != 0)
                    throw new Exception("WSLicenseOpen hr=" + hRes.ToString("X8"));

                Guid g = new Guid();

                if (!ProductIDFromLic(Lic, ref g))
                    throw new Exception("Could not parse ProductID");

                hRes = WSLicenseUninstallLicense(Context, (IntPtr)0, ref g);
                switch(hRes)
                {
                    case 0:
                        break;
                    case 0x80070490:
                        throw new Exception("License not found");
                    default:
                        throw new Exception("WSLicenseUninstallLicense hr=" + hRes.ToString("X8"));
                }

                hRes = WSLicenseClose(Context);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Uninstall license : " + ex.Message);
                if (Context != (IntPtr)0)
                    hRes = WSLicenseClose(Context);
                return false;
            }
            return true;
        }

        public static bool CrackIt(XmlDocument Lic)
        {
            XmlDocument LicOem = ConvertLicToOEM(Lic);
            LicOem.PreserveWhitespace = true;

            if (!InstLic("<License Version=\"1\" Source=\"OEM\" xmlns=\"urn:schemas-microsoft-com:windows:store:licensing:ls\"><Binding Binding_Type=\"Machine\"><ProductID>aaaaaaaa-1111-2222-3333-444444444444</ProductID><PFM>TestPackage_aaaaaaaaaaaaa</PFM></Binding><LicenseInfo Type=\"Full\"><IssuedDate>2022-11-01T16:29:14Z</IssuedDate><LastUpdateDate>2022-11-01T16:29:14Z</LastUpdateDate></LicenseInfo><Signature xmlns=\"http://www.w3.org/2000/09/xmldsig#\"><SignedInfo><CanonicalizationMethod Algorithm=\"http://www.w3.org/2001/10/xml-exc-c14n#\" /><SignatureMethod Algorithm=\"http://www.w3.org/2001/04/xmldsig-more#rsa-sha256\" /><Reference URI=\"\"><Transforms><Transform Algorithm=\"http://www.w3.org/2000/09/xmldsig#enveloped-signature\" /></Transforms><DigestMethod Algorithm=\"http://www.w3.org/2001/04/xmlenc#sha256\" /><DigestValue>UT0YLYT7N0zx8hOrC1+eO9sUDXPNRniF64DH9ppiMGw=</DigestValue></Reference></SignedInfo><SignatureValue>Z4jSCZKb0fMAaH/kf0h2CTuU+QWBv9fhAX1aV21oujgx0QkF0YDaXykR3zlrH/hQU20qFUWTJw69rvBVptTnd4yC/KoKIjobWZK1oojPjBjbzLQkwWfART7uzXsKXXIsLzKk8mNg/6o0WKHBET8hq/JSLAIkT3hxcYIQtGmsCQWJuEBWovjNeuu1TcgIXrXkVpUzsGU6yURqtI/wIPiUEFP+f8XkmxU23cfDdJfOg8kCiQhi3a0JRyu8vn96zuCZXhsGVg8lZd7ighJNklsK6impUdAFJEKxU3vwJLk8wRcvUgcypS0y9E02hZJM4CvTPwLSzx6dWmGmKDcwEEqO1Q==</SignatureValue></Signature></License>",true))
                MessageBox.Show("Test license could not be installed. Did you install WSServiceCrk ?");
            else
            {
                if (InstLic(LicOem,false))
                    MessageBox.Show("Cracked !");
            }

            return true;
        }

        static public void EmptyFolder(DirectoryInfo directoryInfo)
        {
            foreach (FileInfo file in directoryInfo.GetFiles())
                file.Delete();
            foreach (DirectoryInfo subfolder in directoryInfo.GetDirectories())
            {
                EmptyFolder(subfolder);
                subfolder.Delete();
            }
        }
    }
}
