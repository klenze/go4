// $Id$
//-----------------------------------------------------------------------
//       The GSI Online Offline Object Oriented (Go4) Project
//         Experiment Data Processing at EE department, GSI
//-----------------------------------------------------------------------
// Copyright (C) 2000- GSI Helmholtzzentrum fuer Schwerionenforschung GmbH
//                     Planckstr. 1, 64291 Darmstadt, Germany
// Contact:            http://go4.gsi.de
//-----------------------------------------------------------------------
// This software can be used under the license agreements as stated
// in Go4License.txt file which is part of the distribution.
//-----------------------------------------------------------------------

#include "TGo4HttpProxy.h"

#include "TROOT.h"
#include "TClass.h"
#include "TList.h"
#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TBufferFile.h"

#include "TGo4Condition.h"
#include "TGo4Slot.h"
#include "TGo4Iter.h"
#include "TGo4ObjectProxy.h"
#include "TGo4Ratemeter.h"
#include "TGo4AnalysisStatus.h"

#include <QtNetwork>
#include <QTimer>
#include <QElapsedTimer>
#include <QApplication>
#include <QEventLoop>
#include <QInputDialog>

QHttpProxy::QHttpProxy(TGo4HttpProxy* p) :
   QObject(),
   qnam(),
   fHReply(nullptr),
   fProxy(p)
{
   QObject::connect(&qnam, &QNetworkAccessManager::authenticationRequired, this, &QHttpProxy::authenticationRequiredSlot);
}

QHttpProxy::~QHttpProxy()
{
}

void QHttpProxy::httpFinished()
{
   if (!fHReply) return;

   QByteArray res = fHReply->readAll();
   fHReply->deleteLater();
   fHReply = nullptr;
   fProxy->GetHReply(res);
}

void QHttpProxy::httpHReqError(QNetworkReply::NetworkError code)
{
   if (gDebug > 0)
      printf("QHttpProxy::httpHReqError %d %s\n", code, fHReply ? fHReply->errorString().toLatin1().constData() : "---");
   if (fHReply) {
      fHReply->deleteLater();
      fHReply = nullptr;
   }
}

void QHttpProxy::updateRatemeter()
{
   if (fProxy) fProxy->ProcessUpdateTimer();
   QTimer::singleShot(2000, this, &QHttpProxy::updateRatemeter);
}

void QHttpProxy::updateHierarchy()
{
   if (fProxy) fProxy->UpdateHierarchy(kFALSE);
}

void QHttpProxy::regularRequestFinished()
{
   if (fProxy) fProxy->ProcessRegularMultiRequest(kTRUE);
}


void QHttpProxy::StartRequest(const char *url)
{
   fHReply = qnam.get(QNetworkRequest(QUrl(url)));

   QObject::connect(fHReply, &QNetworkReply::finished, this, &QHttpProxy::httpFinished);

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   QObject::connect(fHReply, (void (QNetworkReply::*)(QNetworkReply::NetworkError)) &QNetworkReply::error, this, &QHttpProxy::httpHReqError);
#else
   QObject::connect(fHReply, &QNetworkReply::errorOccurred, this, &QHttpProxy::httpHReqError);
#endif

   QObject::connect(fHReply, &QNetworkReply::sslErrors, [this](const QList<QSslError> &errors) { fHReply->ignoreSslErrors(errors); });

   QSslConfiguration cfg = fHReply->sslConfiguration();
   cfg.setProtocol(QSsl::AnyProtocol/*QSsl::TlsV1SslV3*/);
   fHReply->setSslConfiguration(cfg);
}

void QHttpProxy::authenticationRequiredSlot(QNetworkReply* repl, QAuthenticator* auth)
{
   if (fProxy->fUserName.Length()>0) {
      auth->setUser(fProxy->fUserName.Data());
      auth->setPassword(fProxy->fPassword.Data());
      return;
   }

   bool ok = false;
   QString user_name =
         QInputDialog::getText(0, tr("Authentication required"),
                              tr("User name:"), QLineEdit::Normal,
                              "", &ok);
   if (!ok) return;
   QString passwd =
        QInputDialog::getText(0, tr("Authentication required"),
                              tr("User password:"), QLineEdit::Password,
                              "", &ok);
   if (!ok) return;

   fProxy->fUserName = user_name.toLatin1().constData();
   fProxy->fPassword = passwd.toLatin1().constData();

   auth->setUser(user_name);
   auth->setPassword(passwd);
}

const char *GetHttpRootClassName(const char *kind)
{
   if (!kind || (*kind == 0)) return nullptr;
   if (strncmp(kind,"ROOT.", 5) != 0) return nullptr;
   if (strcmp(kind+5,"TGo4AnalysisWebStatus") == 0)
      return "TGo4AnalysisStatus";
   return kind+5;
}

// ============================================================================================

TGo4HttpAccess::TGo4HttpAccess(TGo4HttpProxy* proxy, XMLNodePointer_t node, Int_t kind, const char *extra_arg) :
   TGo4Access(),
   fProxy(proxy),
   fNode(node),
   fUrlPath(),
   fKind(kind),
   fNameAttr(),
   fKindAttr(),
   fExtraArg(),
   fReceiver(nullptr),
   fRecvPath(),
   fReply(nullptr)
{
   const char *_name = fProxy->fXML->GetAttr(fNode,"_realname");
   if (!_name) _name = fProxy->fXML->GetAttr(fNode,"_name");
   if (_name) fNameAttr = _name;

   fUrlPath = fProxy->MakeUrlPath(node);

   const char *_kind = fProxy->fXML->GetAttr(fNode,"_kind");
   if (_kind) fKindAttr = _kind;
   if (extra_arg) fExtraArg = extra_arg;
}

TClass* TGo4HttpAccess::GetObjectClass() const
{
   if (fKind==3) return TGraph::Class();
   if (fKind==4) return gROOT->GetClass("TGo4ParameterStatus");
   if (fKind==5) return gROOT->GetClass("TTree");
   if (fKind==6) return gROOT->GetClass("TGo4AnalysisStatus");
   if (fKind==7) return gROOT->GetClass("TList");
   if (fKind==8) return gROOT->GetClass("TGo4HistogramStatus");
   return TGo4Proxy::GetClass(GetHttpRootClassName(fKindAttr.Data()));
}

const char *TGo4HttpAccess::GetObjectName() const
{
   return fNameAttr.Data();
}

const char *TGo4HttpAccess::GetObjectClassName() const
{
   if (fKind==3) return "TGraph";
   if (fKind==4) return "TGo4ParameterStatus";
   if (fKind==6) return "TGo4AnalysisStatus";
   if (fKind==7) return "TList";
   if (fKind==8) return "TGo4HistogramStatus";

   const char *clname = GetHttpRootClassName(fKindAttr.Data());

   return clname ? clname : "TObject";
}

Int_t TGo4HttpAccess::AssignObjectTo(TGo4ObjectManager* rcv, const char *path)
{
   if (!rcv) return 0;

   TClass* obj_cl = GetObjectClass();
   if (!obj_cl) {
      printf("TGo4HttpAccess fail to get class %s for object %s\n", GetObjectClassName(), path);
      return 0;
   }

   if (obj_cl->GetBaseClassOffset(TObject::Class()) != 0) {
      printf("TGo4HttpAccess cannot used class %s not derived from TObject\n", GetObjectClassName());
      return 0;
   }

   fReceiver = rcv;
   fRecvPath = path;

   TString url = fProxy->fNodeName;
   if (fUrlPath.Length()>0) { url.Append("/"); url.Append(fUrlPath); }

   switch (fKind) {
      case 0: url.Append("/h.xml?compact"); break;
      case 1: url.Append("/root.bin.gz"); break;
      case 2: url.Append("/get.xml"); break;
      case 3: url.Append("/get.xml.gz?history=100&compact"); break;
      case 4: url.Append("/exe.bin.gz?method=CreateStatus&_destroy_result_"); break;
      case 5: url.Append("/exe.bin.gz?method=CreateSampleTree&sample=0&_destroy_result_"); break;
      case 6: url.Append("/exe.bin.gz?method=CreateStatus&_destroy_result_"); break;
      case 7: url.Append("/exe.bin.gz?method=Select&max=10000"); break;
      case 8: url.Form("%s/Control/go4_sniffer/exe.bin.gz?method=CreateItemStatus&_destroy_result_&itemname=\"%s\"", fProxy->fNodeName.Data(), fUrlPath.Data()); break;
      default: url.Append("/root.bin.gz"); break;
   }

   if (fExtraArg.Length()>0) {
      if (url.Index("?") != kNPOS) url.Append("&"); else url.Append("?");
      url.Append(fExtraArg);
   }

   fReply = fProxy->fComm.qnam.get(QNetworkRequest(QUrl(url.Data())));
   QObject::connect(fReply, &QNetworkReply::finished, this, &TGo4HttpAccess::httpFinished);

#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
   QObject::connect(fReply, (void (QNetworkReply::*)(QNetworkReply::NetworkError)) &QNetworkReply::error, this, &TGo4HttpAccess::httpError);
#else
   QObject::connect(fReply, &QNetworkReply::errorOccurred, this, &TGo4HttpAccess::httpError);
#endif

   if (gDebug > 2)
      printf("TGo4HttpAccess::AssignObjectTo Request URL %s\n", url.Data());

   return 2;
}

void TGo4HttpAccess::httpError(QNetworkReply::NetworkError)
{
   // may do special handling for http errors
}


void TGo4HttpAccess::httpFinished()
{
   QByteArray res = fReply->readAll();
   fReply->deleteLater();
   fReply = nullptr;

   if (gDebug > 2)
      printf("TGo4HttpAccess::httpFinished Get reply size %d\n", res.size());

   // regular ratemeter update used to check connection status
   if (fUrlPath == "Status/Ratemeter") {
      Bool_t conn = res.size() > 0;

      // if proxy in shutdown state - cancel all action in case of communication error
      if (!conn && fProxy->CheckShutdown()) return;

      if (!conn) DoObjectAssignement(fReceiver, fRecvPath.Data(), new TNamed("disconnected","title"), kTRUE); else
      if (fProxy->fConnected != conn) fProxy->UpdateHierarchy(kFALSE);

      fProxy->fConnected = conn;
   }

   // do nothing when nothing received
   if (res.size() == 0) {
      if (gDebug > 0)
         printf("TGo4HttpAccess::httpFinished error with %s\n", fUrlPath.Data());
      return;
   }

   if (fKind == 0) {

      TXMLEngine* xml = fProxy->fXML;

      XMLDocPointer_t doc = xml->ParseString(res.data());

      if (!doc) return;

      XMLNodePointer_t top = xml->GetChild(xml->DocGetRootElement(doc));

      xml->FreeAllAttr(fNode);

      XMLAttrPointer_t attr = xml->GetFirstAttr(top);
      while (attr) {
         xml->NewAttr(fNode, nullptr, xml->GetAttrName(attr), xml->GetAttrValue(attr));
         attr = xml->GetNextAttr(attr);
      }

      XMLNodePointer_t chld;
      while ((chld = xml->GetChild(top)) != nullptr) {
         xml->UnlinkNode(chld);
         xml->AddChild(fNode, chld);
      }

      xml->FreeDoc(doc);

      if (fProxy->fxParentSlot)
         fProxy->fxParentSlot->ForwardEvent(fProxy->fxParentSlot, TGo4Slot::evObjAssigned);
   }

   TObject *obj = nullptr;

   if (fKind == 2) {
      TXMLEngine* xml = fProxy->fXML;

      XMLDocPointer_t doc = xml->ParseString(res.data());
      if (!doc) return;

      XMLNodePointer_t top = xml->DocGetRootElement(doc);

      const char *_kind = xml->GetAttr(top, "_kind");
      const char *_name = xml->GetAttr(top, "_name");
      const char *_title = xml->GetAttr(top, "_title");
      const char *xtitle = xml->GetAttr(top, "xtitle");
      const char *ytitle = xml->GetAttr(top, "ytitle");
      const char *xlabels = xml->GetAttr(top, "xlabels");
      const char *ylabels = xml->GetAttr(top, "ylabels");

      if (strcmp(_kind,"ROOT.TH1D") == 0) {
         Int_t nbins = xml->GetIntAttr(top, "nbins");
         Int_t left = TString(xml->GetAttr(top, "left")).Atof();
         Int_t right = TString(xml->GetAttr(top, "right")).Atof();
         TH1D* h1 = new TH1D(_name, _title, nbins, left, right);
         h1->SetDirectory(nullptr);
         const char *bins = xml->GetAttr(top, "bins") + 1;
         for (int n =-3; n<nbins+2; n++) {
            const char *separ = strpbrk(bins,",]");
            if (!separ) { printf("Error parsing histogram bins\n"); break; }
            if (n >= 0) {
               Double_t v = TString(bins, separ-bins).Atof();
               h1->SetBinContent(n, v);
            }
            bins = separ+1;
         }
         h1->ResetStats();
         obj = h1;
      } else {
         Int_t nbins1 = xml->GetIntAttr(top, "nbins1");
         Int_t left1 = TString(xml->GetAttr(top, "left1")).Atof();
         Int_t right1 = TString(xml->GetAttr(top, "right1")).Atof();
         Int_t nbins2 = xml->GetIntAttr(top, "nbins2");
         Int_t left2 = TString(xml->GetAttr(top, "left2")).Atof();
         Int_t right2 = TString(xml->GetAttr(top, "right2")).Atof();
         TH2D* h2 = new TH2D(_name, _title, nbins1, left1, right1, nbins2, left2, right2);
         h2->SetDirectory(nullptr);
         const char *bins = xml->GetAttr(top, "bins") + 1;
         for (int n =-6; n<(nbins1+2)*(nbins2+2); n++) {
            const char *separ = strpbrk(bins,",]");
            if (!separ) { printf("Error parsing histogram bins\n"); break; }
            if (n >= 0) {
               Double_t v = TString(bins, separ-bins).Atof();
               h2->SetBinContent(n % (nbins1 + 2), n / (nbins1 + 2),  v);
            }
            bins = separ+1;
         }
         h2->ResetStats();
         obj = h2;
      }

      if (obj) {
         if (xtitle)
           ((TH1*)obj)->GetXaxis()->SetTitle(xtitle);
         if (ytitle)
           ((TH1*)obj)->GetYaxis()->SetTitle(ytitle);
         if (xlabels) {
            TObjArray* arr = TString(xlabels).Tokenize(",");
            for (int n = 0; n <= (arr ? arr->GetLast() : -1); n++)
               ((TH1 *)obj)->GetXaxis()->SetBinLabel(1 + n, arr->At(n)->GetName());
            delete arr;
         }
         if (ylabels) {
            TObjArray* arr = TString(ylabels).Tokenize(",");
            for (int n = 0; n <= (arr ? arr->GetLast() : -1); n++)
               ((TH1 *)obj)->GetYaxis()->SetBinLabel(1 + n, arr->At(n)->GetName());
            delete arr;
         }
      }

      xml->FreeDoc(doc);

   } else

   if (fKind == 3) {
      TXMLEngine* xml = fProxy->fXML;

      XMLDocPointer_t doc = xml->ParseString(res.data());
      if (!doc) return;

      XMLNodePointer_t top = xml->DocGetRootElement(doc);

      XMLNodePointer_t chld = top;
      Int_t cnt = 0;
      while (chld) {
         if (xml->GetAttr(chld, "value") && xml->GetAttr(chld, "time")) cnt++;
         chld = (chld==top) ? xml->GetChild(top) : xml->GetNext(chld);
      }

      TGraph* gr = new TGraph(cnt);
      gr->SetName(xml->GetAttr(top, "_name"));
      gr->SetTitle(TString::Format("%s ratemeter", xml->GetAttr(top, "_name")).Data());

      chld = top;
      Int_t i = cnt-1;
      while (chld) {
         const char *time = xml->GetAttr(chld, "time");
         const char *value = xml->GetAttr(chld, "value");
         if (time && value) {
            QDateTime tm = QDateTime::fromString(time, Qt::ISODate);
            gr->SetPoint(i, tm.toSecsSinceEpoch(), TString(value).Atof());
            i = (i+1) % cnt;
         }
         chld = (chld==top) ? xml->GetChild(top) : xml->GetNext(chld);
      }

      xml->FreeDoc(doc);
      obj = gr;

      gr->GetXaxis()->SetTimeDisplay(1);
      gr->GetXaxis()->SetTimeFormat("%H:%M:%S%F1970-01-01 00:00:00");
   } else {

      TClass* obj_cl = GetObjectClass();

      if (gDebug > 2) printf("TGo4HttpAccess::httpFinished Reconstruct object class %s\n", obj_cl ? obj_cl->GetName() : "---");

      if (!obj_cl || (obj_cl->GetBaseClassOffset(TObject::Class()) != 0)) return;

      obj = (TObject*) obj_cl->New();
      if (!obj) {
         printf("TGo4HttpAccess fail to create object of class %s\n", GetObjectClassName());
         return;
      }

      TBufferFile buf(TBuffer::kRead, res.size(), res.data(), kFALSE);
      buf.MapObject(obj, obj_cl);

      obj->Streamer(buf);

      // workaround - when ratemeter received, check running state
      if (obj->IsA() == TGo4Ratemeter::Class())
         fProxy->fbAnalysisRunning = ((TGo4Ratemeter*) obj)->IsRunning();

      if (fKind == 6)
         fProxy->SetAnalysisReady(kTRUE);

   }

   DoObjectAssignement(fReceiver, fRecvPath.Data(), obj, kTRUE);
}

// =========================================================================

class TGo4HttpLevelIter : public TGo4LevelIter {
   protected:
      TXMLEngine      *fXML;
      XMLNodePointer_t fParent;
      XMLNodePointer_t fChild;

   public:
      TGo4HttpLevelIter(TXMLEngine* xml, XMLNodePointer_t item) :
         TGo4LevelIter(),
         fXML(xml),
         fParent(item),
         fChild()
         {
         }

      virtual ~TGo4HttpLevelIter() {}

      Bool_t next() override
      {
         if (!fParent) return kFALSE;

         while (true) {

            if (!fChild) {
               fChild = fXML->GetChild(fParent);
            } else {
               fChild = fXML->GetNext(fChild);
            }

            if (!fChild) return kFALSE;

            if (fXML->HasAttr(fChild,"_hidden")) continue;

            break;
         }

         return fChild != nullptr;
      }

      Bool_t isfolder() override
      {
         return fXML->GetChild(fChild) != nullptr;
      }

      Int_t getflag(const char *flagname) override
      {
         if (strcmp(flagname,"IsRemote") == 0) return 1;

         if (strcmp(flagname,"IsDeleteProtect") == 0)
            return fXML->HasAttr(fChild, "_can_delete") ? 0 : 1;

         if (strcmp(flagname,"IsResetProtect") == 0)
            return fXML->HasAttr(fChild, "_no_reset") ? 1 : 0;

         if (strcmp(flagname,"_numargs") == 0) {
            const char *_numargs = fXML->GetAttr(fChild,"_numargs");
            return !_numargs ? -1 : TString(_numargs).Atoi();
         }

         return -1;
      }

      TGo4LevelIter* subiterator() override
      {
         if (!isfolder()) return nullptr;
         return new TGo4HttpLevelIter(fXML,fChild);
      }

      TGo4Slot* getslot() override { return nullptr; }

      const char *name() override
      {
         const char *real = fXML->GetAttr(fChild,"_realname");
         return real? real : fXML->GetAttr(fChild,"_name");
      }

      const char *info() override { return fXML->GetAttr(fChild,"_title"); }
      Int_t sizeinfo() override { return 0; }

      Int_t GetKind() override
      {
         if (isfolder()) return TGo4Access::kndFolder;

         if (fXML->HasAttr(fChild,"_go4event")) return TGo4Access::kndEventElement;

         if (fXML->HasAttr(fChild,"_more")) return TGo4Access::kndMoreFolder;

         const char *_kind = fXML->GetAttr(fChild,"_kind");
         if (_kind && strcmp(_kind,"Command") == 0) return TGo4Access::kndRootCommand;

         const char *drawfunc = fXML->GetAttr(fChild,"_drawfunc");
         if (drawfunc && !strcmp(drawfunc, "GO4.drawParameter")) return TGo4Access::kndGo4Param;

         const char *classname = GetClassName();

         if (!classname) return TGo4Access::kndNone;

         if (strcmp(classname,"TLeafElement") == 0) return TGo4Access::kndTreeLeaf;

         return TGo4Access::kndObject;
      }

      const char *GetClassName() override
      {
         const char *_kind = fXML->GetAttr(fChild,"_kind");
         const char *res = GetHttpRootClassName(_kind);
         if (res) return res;
         if (_kind && !strcmp(_kind,"rate") && fXML->HasAttr(fChild,"_history")) return "TGraph";
         return isfolder() ? "TFolder" : nullptr;
      }
};

// =====================================================================

TGo4HttpProxy::TGo4HttpProxy() :
   TGo4ServerProxy(),
   fXML(nullptr),
   fxHierarchy(nullptr),
   fComm(this),
   fRateCnt(0),
   fStatusCnt(0),
   fDebugCnt(0),
   fbAnalysisRunning(kFALSE),
   fUserName(),
   fPassword(),
   fConnected(kTRUE),
   fRegularReq(nullptr)
{
   fXML = new TXMLEngine;
   fUserName = "anonymous";
   // SetAccount("observer","go4view");
   // SetAccount("controller","go4ctrl");
}

TGo4HttpProxy::~TGo4HttpProxy()
{
   fXML->FreeDoc(fxHierarchy);
   fxHierarchy = nullptr;

   delete fXML; fXML = nullptr;
}

void TGo4HttpProxy::SetAccount(const char *username, const char *passwd)
{
   fUserName = username ? username : "";
   fPassword = passwd ? passwd : "";
}

void TGo4HttpProxy::Initialize(TGo4Slot* slot)
{
   TGo4ServerProxy::Initialize(slot);

   if (!IsGo4Analysis()) return;

   TGo4Slot* subslot = new TGo4Slot(fxParentSlot, "Settings", "Analysis configuration");
   subslot->SetProxy(new TGo4ObjectProxy());

   subslot = new TGo4Slot(fxParentSlot, "Ratemeter", "Analysis ratemeter");
   subslot->SetProxy(new TGo4ObjectProxy());

   subslot = new TGo4Slot(fxParentSlot, "Loginfo", "Latest status messages");
   subslot->SetProxy(new TGo4ObjectProxy());

   subslot = new TGo4Slot(fxParentSlot, "Debugoutput", "Debug output of go4 analysis");
   subslot->SetProxy(new TGo4ObjectProxy());

   QTimer::singleShot(2000, &fComm, &QHttpProxy::updateRatemeter);
}


XMLNodePointer_t TGo4HttpProxy::FindItem(const char *name, XMLNodePointer_t curr) const
{
   if (!curr) curr = fXML->GetChild(fXML->DocGetRootElement(fxHierarchy));

   if (!curr || !name || (*name == 0)) return curr;

   const char *slash = strchr(name,'/');
   bool doagain = false;

   do {
      int len = !slash ? strlen(name) : (slash - name);

      XMLNodePointer_t chld = fXML->GetChild(curr);
      while (chld) {
         const char *_name = fXML->GetAttr(chld,"_realname");
         if (!_name) _name = fXML->GetAttr(chld,"_name");

         if (_name && (strlen(_name) == len) && (strncmp(_name, name, len) == 0))
            return FindItem(slash ? slash+1 : nullptr, chld);

         chld = fXML->GetNext(chld);
      }

      // we try to process situation when item name contains slashes
      doagain = slash != nullptr;
      if (slash) slash = strchr(slash+1,'/');

   } while (doagain);

   return nullptr;
}

TString TGo4HttpProxy::MakeUrlPath(XMLNodePointer_t item)
{
   if (!item) return TString("");

   XMLNodePointer_t root = fXML->GetChild(fXML->DocGetRootElement(fxHierarchy));

   TString res;

   while (item != root) {
      const char *_name = fXML->GetAttr(item,"_name");
      if (!_name) return TString("");
      if (res.Length()>0)
         res = TString(_name) + "/" + res;
      else
         res = _name;

      item = fXML->GetParent(item);
      if (!item) return TString("");
   }

   return res;
}


void TGo4HttpProxy::GetHReply(QByteArray& res)
{
   if (res.size() == 0) return;
   XMLDocPointer_t doc = fXML->ParseString(res.data());

   if (doc) {
      fXML->FreeDoc(fxHierarchy);
      fxHierarchy = doc;
   }

   if (fxParentSlot)
     fxParentSlot->ForwardEvent(fxParentSlot, TGo4Slot::evObjAssigned);
}

const char *TGo4HttpProxy::GetContainedObjectInfo()
{
   TGo4ServerProxy::GetContainedObjectInfo(); // evaluate roles
   fInfoStr +="(";
   fInfoStr +=GetUserName();
   fInfoStr +="@";
   fInfoStr +=GetServerName();
   fInfoStr +=")";
   const char *analname = fXML->GetAttr(FindItem(""), "_analysis_name");
   if (analname) {
      fInfoStr += " name:";
      fInfoStr += analname;
   }

   return fInfoStr.Data();
}

Bool_t TGo4HttpProxy::Connect(const char *nodename)
{
   fNodeName = nodename;

   if ((fNodeName.Index("http://") != 0)  && (fNodeName.Index("https://") != 0))
      fNodeName = TString("http://") + fNodeName;

   return UpdateHierarchy(kTRUE);
}

Bool_t TGo4HttpProxy::NamesListReceived()
{
   return fxHierarchy && !fComm.fHReply;
}

Bool_t TGo4HttpProxy::UpdateHierarchy(Bool_t sync)
{
   if (fComm.fHReply) return kTRUE;

   TString req = fNodeName + "/h.xml?compact";

   fComm.StartRequest(req.Data());

   if (!sync) return kTRUE;

   QElapsedTimer t;
   t.start();

   // wait several seconds
   while (t.elapsed() < 5000) {
      if (!fComm.fHReply) break;
      QApplication::processEvents();
   }

   return fxHierarchy != nullptr;
}

Bool_t TGo4HttpProxy::HasSublevels() const
{
   return fxHierarchy != nullptr;
}

TGo4Access* TGo4HttpProxy::ProvideAccess(const char *name)
{
   XMLNodePointer_t item = FindItem(name);

   if (!item) return nullptr;

   const char *_kind = fXML->GetAttr(item,"_kind");

   Int_t kind = 1;

   if (!strcmp(_kind, "rate") && fXML->HasAttr(item, "_history"))
      kind = 3;
   else if (fXML->HasAttr(item, "_dabc_hist"))
      kind = 2;
   else if (fXML->HasAttr(item, "_more"))
      kind = 0;

   return new TGo4HttpAccess(this, item, kind);
}

TGo4LevelIter* TGo4HttpProxy::MakeIter()
{
   if (!fxHierarchy) return nullptr;
   XMLNodePointer_t top = fXML->GetChild(fXML->DocGetRootElement(fxHierarchy));

   return !top ? nullptr : new TGo4HttpLevelIter(fXML, top);
}

void TGo4HttpProxy::WriteData(TGo4Slot* slot, TDirectory* dir, Bool_t onlyobjs)
{
}

void TGo4HttpProxy::ReadData(TGo4Slot* slot, TDirectory* dir)
{
}

void TGo4HttpProxy::Update(TGo4Slot* slot, Bool_t strong)
{
   if (strong)
      UpdateHierarchy(kFALSE);
}

Bool_t TGo4HttpProxy::ServerHasRestrict()
{
   // return kTRUE when server has Restrict methods
   // It is indication of new functionality like commands with arguments or support of POST requests

   return fXML->HasAttr(FindItem(""),"_has_restrict");
}

Bool_t TGo4HttpProxy::ServerHasMulti()
{
   // return kTRUE when server has ProduceMulti method
   // One could use it to submit many requests at once

   return fXML->HasAttr(FindItem(""),"_has_produce_multi");
}


Bool_t TGo4HttpProxy::IsGo4Analysis() const
{
   XMLNodePointer_t item = FindItem("");
   if (!item) return kFALSE;

   const char *_kind = fXML->GetAttr(item,"_kind");
   const char *_title = fXML->GetAttr(item,"_title");

   if (!_kind || !_title) return kFALSE;

   return !strcmp(_kind,"ROOT.Session") && !strcmp(_title,"GO4 analysis");
}

Bool_t TGo4HttpProxy::CheckUserName(const char *expects, Bool_t dflt)
{
   XMLNodePointer_t item = FindItem("");
   if (!item) return dflt;

   const char *username = fXML->GetAttr(item,"_username");
   if (!username) return dflt;

   return strcmp(username, expects) == 0;
}


Bool_t TGo4HttpProxy::RefreshNamesList()
{
   return UpdateHierarchy(kFALSE);
}

Bool_t TGo4HttpProxy::DelayedRefreshNamesList(Int_t delay_sec)
{
   QTimer::singleShot(delay_sec*1000, &fComm, &QHttpProxy::updateHierarchy);

   return kTRUE;
}

Bool_t TGo4HttpProxy::CanSubmitObjects()
{
   if (!IsGo4Analysis() || IsViewer()) return kFALSE;

   return ServerHasRestrict();
}


void TGo4HttpProxy::RequestAnalysisSettings()
{
   if (SubmitRequest("Control/Analysis", 6, SettingsSlot()))
   SetAnalysisSettingsReady(kTRUE);  // workaround - mark as we finished with settings
}

void TGo4HttpProxy::SubmitAnalysisSettings()
{
   TGo4AnalysisStatus* status = nullptr;
   if (SettingsSlot())
      status = dynamic_cast<TGo4AnalysisStatus*>(SettingsSlot()->GetAssignedObject());

   if (status)
      PostObject("Control/Analysis/exe.bin?method=ApplyStatus&status", status, 2);
}

void TGo4HttpProxy::CloseAnalysisSettings()
{
   SetAnalysisSettingsReady(kFALSE);

   SubmitCommand("Control/CmdClose");
}

void TGo4HttpProxy::ClearAllAnalysisObjects()
{
   // when command submitted without arguments, histograms and conditions folder will be cleared
   SubmitCommand("Control/CmdClear");
}

void TGo4HttpProxy::ClearAnalysisObject(const char *fullpath)
{
   TString foldername, objectname;
   TGo4Slot::ProduceFolderAndName(fullpath, foldername, objectname);

   objectname = TString("\"") + objectname + TString("\"");

   SubmitCommand("Control/CmdClearObject", -1, objectname.Data());
}

void TGo4HttpProxy::RemoveObjectFromAnalysis(const char *fullpath)
{
   TString foldername, objectname;
   TGo4Slot::ProduceFolderAndName(fullpath, foldername, objectname);

   objectname = TString("\"") + objectname + TString("\"");

   SubmitCommand("Control/CmdDeleteObject", -1, objectname.Data());
}

void TGo4HttpProxy::ExecuteLine(const char *line)
{
   // use method of TGo4AnalysisWebStatus - this works with all THttpServer versions
   if (FindItem("Control/Analysis"))
      SubmitURL(TString::Format("Control/Analysis/exe.json?method=ExecuteLine&cmd=\"%s\"", line));
}

void TGo4HttpProxy::StartAnalysis()
{
   SubmitCommand("Control/CmdStart");
   fbAnalysisRunning = kTRUE;
}

void TGo4HttpProxy::StopAnalysis()
{
   SubmitCommand("Control/CmdStop");
   fbAnalysisRunning = kFALSE;
}

Bool_t TGo4HttpProxy::RequestObjectStatus(const char *objectname, TGo4Slot* tgtslot)
{
   return SubmitRequest(objectname, 4, tgtslot) != nullptr;
}

Bool_t TGo4HttpProxy::SubmitURL(const char *path, Int_t waitres)
{
   TString url = fNodeName;
   url.Append("/");
   url.Append(path);

   if (gDebug > 1) printf("Submit URL %s\n", url.Data());

   QNetworkReply *netReply = fComm.qnam.get(QNetworkRequest(QUrl(url.Data())));

   QSslConfiguration cfg = netReply->sslConfiguration();
   cfg.setProtocol(QSsl::AnyProtocol/*QSsl::TlsV1SslV3*/);
   netReply->setSslConfiguration(cfg);

   if (waitres <= 0) {
      QObject::connect(netReply, &QNetworkReply::finished, netReply, &QNetworkReply::deleteLater);
      return kTRUE;
   }

   QEventLoop loop;
   QElapsedTimer myTimer;
   myTimer.start();
   while (!netReply->isFinished()) {
      loop.processEvents(QEventLoop::AllEvents,100);
      if (myTimer.elapsed() > waitres*1000) break;
   }

   netReply->deleteLater();
   return netReply->isFinished();
}

TString TGo4HttpProxy::FindCommand(const char *name)
{
   if (!name || (*name == 0)) return "";
   if (NumCommandArgs(name) >= 0) return name;

   TGo4Iter iter(fxParentSlot);

   while (iter.next()) {
      if (iter.getflag("_numargs") < 0) continue;
      if (strcmp(iter.getname(),name) == 0) return iter.getfullname();
   }

   return "";
}

Int_t TGo4HttpProxy::NumCommandArgs(const char *name)
{
   XMLNodePointer_t item = FindItem(name);
   if (!item) return -1;

   const char *_numargs = fXML->GetAttr(item,"_numargs");
   if (!_numargs) return 0;

   return TString(_numargs).Atoi();
}

Bool_t TGo4HttpProxy::SubmitCommand(const char *name, Int_t waitres, const char *arg1, const char *arg2, const char *arg3)
{
   TString url(name);
   url.Append("/cmd.json");
   if (arg1 && *arg1) {
      url.Append("?arg1=");
      url.Append(arg1);
      if (arg2 && *arg2) {
         url.Append("&arg2=");
         url.Append(arg2);
         if (arg3 && *arg3) {
            url.Append("&arg3=");
            url.Append(arg3);
         }
      }
   }

   return SubmitURL(url.Data(), waitres);
}

Bool_t TGo4HttpProxy::PostObject(const char *prefix, TObject *obj, Int_t waitres, Bool_t destroy_after)
{
   if (!ServerHasRestrict()) return kFALSE;

   TBufferFile *sbuf = new TBufferFile(TBuffer::kWrite, 100000);
   sbuf->MapObject(obj);
   obj->Streamer(*sbuf);

   QByteArray postData;
   postData.append(sbuf->Buffer(), sbuf->Length());

   delete sbuf;

   TString url = fNodeName;
   url.Append("/");
   url.Append(prefix);
   url.Append("=_post_object_&");
   if (destroy_after) url.Append("_destroy_post_&");
   url.Append("_post_class_=");
   url.Append(obj->ClassName());

   // printf("URL %s datalen %d\n", url.Data(), postData.length());

   QNetworkRequest req(QUrl(url.Data()));

   QNetworkReply *netReply = fComm.qnam.post(req, postData);

   QSslConfiguration cfg = netReply->sslConfiguration();
   cfg.setProtocol(QSsl::AnyProtocol/*QSsl::TlsV1SslV3*/);
   netReply->setSslConfiguration(cfg);

   if (waitres <= 0) {
      QObject::connect(netReply, &QNetworkReply::finished, netReply, &QNetworkReply::deleteLater);
      return kTRUE;
   }

   QEventLoop loop;
   QElapsedTimer myTimer;
   myTimer.start();
   while (!netReply->isFinished()) {
      loop.processEvents(QEventLoop::AllEvents,100);
      if (myTimer.elapsed() > waitres*1000) break;
   }

   netReply->deleteLater();
   return netReply->isFinished();
}


Bool_t TGo4HttpProxy::UpdateAnalysisObject(const char *objectname, TObject *obj)
{
   TString url;

   Bool_t destr = kTRUE;

   if (*objectname == 0) {
      url.Append("Control/go4_sniffer/exe.bin?method=AddAnalysisObject&obj");
      destr = kFALSE; // object will be owned by analysis
   } else {
      url.Append(objectname);
      url.Append("/exe.bin?method=");

      if (obj->InheritsFrom(TGo4Condition::Class())) {
         url.Append("UpdateFrom&counts=kFALSE&cond");
      } else {
         url.Append("SetStatus&status");
      }
   }

   return PostObject(url.Data(), obj, 2, destr);
}

void TGo4HttpProxy::ResetDebugOutputRequests()
{
   TGo4Slot* subslot = DebugOutputSlot();
   if (subslot) subslot->AssignObject(0, kFALSE);
}

void TGo4HttpProxy::ProcessRegularMultiRequest(Bool_t finished)
{
   if (finished) {
      if (!fRegularReq) return;

      QByteArray res = fRegularReq->readAll();
      fRegularReq->deleteLater();
      fRegularReq = nullptr;

      if (res.size() <= 0) {
         fConnected = false;

         // check if proxy is in shutdown phase
         if (CheckShutdown()) return;

         RatemeterSlot()->AssignObject(new TNamed("disconnected","title"),kTRUE);

         return;
      }

      if (!fConnected) {
         UpdateHierarchy(kFALSE);
         fConnected = true;
      }

      if (gDebug>2) printf("TGo4HttpProxy:: get reply on multi.bin request with %d bytes\n", res.size());

      int pos = 0;

      for (int n = 0; n < 3; n++) {
         unsigned char *ptr = (unsigned char *)res.data() + pos;
         unsigned len =  ((unsigned) ptr[0]) |
                        (((unsigned) ptr[1]) << 8) |
                        (((unsigned) ptr[2]) << 16) |
                        (((unsigned) ptr[3]) << 24);

         ptr += 4;
         pos += 4 + len;
         if (len > res.size()) {
            printf("Error decoding multi.bin buffer\n");
            return;
         }

         if (gDebug>2) printf("Decoding portion of %d bytes\n", len);

         TClass* obj_cl = n == 0 ? TGo4Ratemeter::Class() : TList::Class();
         TObject *obj = (TObject*) obj_cl->New();

         TBufferFile buf(TBuffer::kRead, len, ptr, kFALSE);
         buf.MapObject(obj, obj_cl);

         obj->Streamer(buf);

         if (n > 0) {
            TGo4Slot* subslot = n==1 ? LoginfoSlot() : DebugOutputSlot();
            TList* curr = subslot ? dynamic_cast<TList*> (subslot->GetAssignedObject()) : nullptr;
            TList* next = dynamic_cast<TList*> (obj);
            if (curr && curr->First() && next && next->First()) {
               if (strcmp(curr->First()->GetName(), next->First()->GetName()) == 0) {
                  // this is protection against sending same info many times
                  // happend with sever snapshot
                  delete obj;
                  obj = nullptr;
               }
            }
         }

         switch(n) {
            case 0:
               fbAnalysisRunning = ((TGo4Ratemeter*) obj)->IsRunning();
               RatemeterSlot()->AssignObject(obj,kTRUE);
               break;
            case 1:
               if (obj) LoginfoSlot()->AssignObject(obj,kTRUE);
               break;
            case 2:
               if (obj) DebugOutputSlot()->AssignObject(obj,kTRUE);
               break;
         }
      }

      if (pos != res.size()) printf("Decoding fails %d != %d bytes\n", pos, res.size());

      return;
   }


   if (fRegularReq) return;

   TString req;

   req.Append("Ratemeter/root.bin\n");

   req.Append("Msg/exe.bin?method=Select&max=10000");
   TGo4Slot* subslot = LoginfoSlot();
   TList* curr = subslot ? dynamic_cast<TList*> (subslot->GetAssignedObject()) : nullptr;
   if (curr && curr->First()) {
      req.Append("&id=");
      req.Append(curr->First()->GetName());

   }
   req.Append("\n");

   req.Append("Log/exe.bin?method=Select&max=10000");
   subslot = DebugOutputSlot();
   curr = subslot ? dynamic_cast<TList*> (subslot->GetAssignedObject()) : nullptr;
   if (curr && curr->First()) {
      req.Append("&id=");
      req.Append(curr->First()->GetName());

   }
   req.Append("\n");

   QByteArray postData;
   postData.append(req.Data(), req.Length());
   if (gDebug>2) printf("Sending multi.bin request\n%s", req.Data());

   TString url = fNodeName;
   url.Append("/Status/multi.bin.gz?number=3");

   fRegularReq = fComm.qnam.post(QNetworkRequest(QUrl(url.Data())), postData);

   QSslConfiguration cfg = fRegularReq->sslConfiguration();
   cfg.setProtocol(QSsl::AnyProtocol/*QSsl::TlsV1SslV3*/);
   fRegularReq->setSslConfiguration(cfg);

   QObject::connect(fRegularReq, &QNetworkReply::finished, &fComm, &QHttpProxy::regularRequestFinished);
}

Bool_t TGo4HttpProxy::CheckShutdown(Bool_t force)
{
   if (force || (fShutdownCnt>0)) {
      if (fxParentSlot) fxParentSlot->Delete();
      fxParentSlot = nullptr;
      return kTRUE;
   }
   return kFALSE;
}


void TGo4HttpProxy::ProcessUpdateTimer()
{
   if ((fShutdownCnt > 0) && (--fShutdownCnt == 0)) {
      CheckShutdown(kTRUE);
      return;
   }

   if (ServerHasMulti() || fRegularReq) {
      ProcessRegularMultiRequest();
      return;
   }

   TGo4Slot* subslot = RatemeterSlot();
   if (subslot) {
      // no new update since last call
      if (!subslot->GetAssignedObject() || (fRateCnt != subslot->GetAssignCnt())) {
         fRateCnt = subslot->GetAssignCnt();
         SubmitRequest("Status/Ratemeter", 1, subslot);
      }
   }

   subslot = LoginfoSlot();
   if (subslot && IsConnected()) {
      TList* curr = dynamic_cast<TList*> (subslot->GetAssignedObject());
      if (!curr || (fStatusCnt != subslot->GetAssignCnt())) {
         TString arg;
         if (curr && curr->First()) {
            arg = "id=";
            arg += curr->First()->GetName();
         }
         fStatusCnt = subslot->GetAssignCnt();
         SubmitRequest("Status/Msg", 7, subslot, arg);
      }
   }

   subslot = DebugOutputSlot();
   if (subslot && IsConnected()) {
      TList* curr = dynamic_cast<TList*> (subslot->GetAssignedObject());
      if (!curr || (fDebugCnt != subslot->GetAssignCnt())) {
         TString arg;
         if (curr && curr->First()) {
            arg = "id=";
            arg += curr->First()->GetName();
         }
         fDebugCnt = subslot->GetAssignCnt();
         SubmitRequest("Status/Log", 7, subslot, arg);
      }
   }

}

void TGo4HttpProxy::RemoteTreeDraw(const char *treename,
                                   const char *varexp,
                                   const char *cutcond,
                                   const char *hname)
{
   TString tfoldername, tobjectname;
   TGo4Slot::ProduceFolderAndName(treename, tfoldername, tobjectname);

   TString hfoldername, hobjectname;
   TGo4Slot::ProduceFolderAndName(hname, hfoldername, hobjectname);

   TString path;
   path.Form("Control/go4_sniffer/exe.json?method=RemoteTreeDraw&histoname=\"%s\"&treename=\"%s\"&varexpr=\"%s\"&cutexpr=\"%s\"",
         hobjectname.Data(), tobjectname.Data(), varexp, cutcond);

   SubmitURL(path, 2);
}

TGo4HttpAccess* TGo4HttpProxy::SubmitRequest(const char *itemname, Int_t kind, TGo4Slot* tgtslot, const char *extra_arg)
{
   if (!itemname || !tgtslot) return nullptr;

   XMLNodePointer_t item = FindItem(itemname);
   if (!item) return nullptr;

   if (kind == 4) {
      // when status for histogram is requested, redirect request to the sniffer
      const char *_objkind = fXML->GetAttr(item, "_kind");
      if (_objkind && ((strstr(_objkind, "ROOT.TH1") == _objkind) || (strstr(_objkind, "ROOT.TH2") == _objkind) ||
                       (strstr(_objkind, "ROOT.TH3") == _objkind)))
         kind = 8;
   }

   TGo4HttpAccess* access = new TGo4HttpAccess(this, item, kind, extra_arg);
   access->AssignObjectToSlot(tgtslot); // request event itself

   return access;
}


void TGo4HttpProxy::RequestEventStatus(const char *evname, Bool_t astree, TGo4Slot* tgtslot)
{
   if (!tgtslot) {
      // this is remote printing of event

      TString url = evname;
      url.Append("/exe.bin?method=");
      url.Append(astree ? "ShowSampleTree" : "PrintEvent");

      SubmitURL(url);
      return;
   }

   SubmitRequest(evname, astree ? 5 : 1, tgtslot);
}

void TGo4HttpProxy::RemotePrintEvent(const char *evname,
                                     Int_t evnumber,
                                     Int_t subid,
                                     Bool_t ishex,
                                     Bool_t islong)
{
   TString url;
   url.Form("Events/%s/exe.bin?method=SetPrintEvent&num=%d&sid=%d&longw=%d&hexw=%d&dataw=%d",
              evname, evnumber, subid, islong ? 1 : 0, ishex ? 1 : 0, ishex ? 0 : 1);
   SubmitURL(url);
}

void TGo4HttpProxy::ChageObjectProtection(const char *fullpath, const char *flags)
{
   unsigned reset_bits = 0, set_bits = 0;

   TString opt = flags;
   if(opt.Contains("+D")) reset_bits |= TGo4Status::kGo4CanDelete;
   if(opt.Contains("-D")) set_bits |= TGo4Status::kGo4CanDelete;
   if(opt.Contains("+C")) set_bits |= TGo4Status::kGo4NoReset;
   if(opt.Contains("-C")) reset_bits |= TGo4Status::kGo4NoReset;

   TString url(fullpath);
   url.Append("/exe.bin?method=");

   if (reset_bits != 0) SubmitURL(url + TString::Format("ResetBit&f=%u",reset_bits));
   if (set_bits != 0) SubmitURL(url + TString::Format("SetBit&f=%u&prototype=UInt_t",set_bits));
}

void TGo4HttpProxy::PrintDynListEntry(const char *fullpath)
{
   SubmitURL(TString(fullpath)+"/exe.bin?method=Print");
}

void TGo4HttpProxy::LoadConfigFile(const char *fname)
{
   SubmitURL(TString::Format("Control/Analysis/exe.bin?method=LoadStatus&fname=%s", fname));
}

void TGo4HttpProxy::SaveConfigFile(const char *fname)
{
   SubmitURL(TString::Format("Control/Analysis/exe.bin?method=SaveStatus&fname=%s", fname));
}

void TGo4HttpProxy::WriteAutoSave(const char *fname,
                                  Int_t complevel,
                                  Bool_t overwrite)
{
   SubmitURL(TString::Format("Control/Analysis/exe.bin?method=WriteAutoSave&fname=%s&overwrite=%s&complevel=%d", fname, overwrite ? "kTRUE" : "kFALSE", complevel));
}

void TGo4HttpProxy::DisconnectAnalysis(Int_t waittime, Bool_t servershutdown)
{
   if (servershutdown && IsGo4Analysis() && IsAdministrator()) {
      SubmitCommand("Control/CmdExit");
      fShutdownCnt = waittime;
   } else {
      CheckShutdown(kTRUE);
   }
}


