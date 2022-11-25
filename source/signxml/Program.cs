using System;
using System.Collections.Generic;
using System.Text;
using System.IO;
using System.Xml;
using System.Security.Cryptography;
using System.Security.Cryptography.Xml;


namespace signxml
{
    class Program
    {
        public static void SignXml(XmlDocument xmlDoc,RSA key)
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


        static bool RemoveSignature(XmlDocument x)
        {
            XmlNode nodeRoot = x.ChildNodes[0];
            if (nodeRoot == null)
                return false;
            XmlNode nodeSignature = nodeRoot["Signature"];
            if (nodeSignature == null)
                return false;
            nodeRoot.RemoveChild(nodeSignature);
            return true;
        }

        static void FixSignature(XmlDocument x)
        {
            RemoveSignature(x);
            SignXmlWithOurKey(x);
        }

        static void Main(string[] args)
        {
            if (args.Length < 2)
            {
                Console.WriteLine("signxml <input_file> <output_file>\n");
                return;
            }


            try
            {
                CryptoConfig.AddAlgorithm(typeof(Security.Cryptography.RSAPKCS1SHA256SignatureDescription), "http://www.w3.org/2001/04/xmldsig-more#rsa-sha256");

                string inXmlText = File.ReadAllText(args[0]);

                XmlDocument x = new XmlDocument();
                x.PreserveWhitespace = true;
                x.InnerXml = inXmlText;

                FixSignature(x);

                File.WriteAllText(args[1], x.InnerXml);

            }
            catch (Exception ex)
            {
                Console.WriteLine("Error: {0}", ex.Message);
            }

        }
    }
}
