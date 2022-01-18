// $Id$

JSROOT.define(["painter", "jquery", "jquery-ui"], (jsrp, $) => {

   if (typeof JSROOT != "object") {
      let e1 = new Error("analysiseditor.js requires JSROOT to be already loaded");
      e1.source = "analysiseditor.js";
      throw e1;
   }

   if (typeof GO4 != "object") {
      let e1 = new Error("analysiseditor.js requires GO4 to be already loaded");
      e1.source = "analysiseditor.js";
      throw e1;
   }

   // only during transition
   JSROOT.loadScript("https://root.cern/js/6.3.2/style/jquery-ui.min.css");


   GO4.EvIOType = {
          GO4EV_NULL: 0,                // no event store/source
          GO4EV_FILE: 1,                // root file with own tree
          GO4EV_TREE: 2,                // branch of singleton tree
          GO4EV_MBS_FILE: 3,            // mbs listmode file (input only)
          GO4EV_MBS_STREAM: 4,          // mbs stream server (input only)
          GO4EV_MBS_TRANSPORT: 5,       // mbs transport server (input only)
          GO4EV_MBS_EVENTSERVER: 6,     // mbs event server  (input only)
          GO4EV_MBS_REVSERV: 7,         // remote event server (input only)
          GO4EV_BACK: 8,                // backstore in memory (pseudo-ringbuffer?)
          GO4EV_USER: 9,                // user defined source class
          GO4EV_MBS_RANDOM: 10,         // random generated mbs event
          GO4EV_HDF5: 11                // HDF5 file format
      };

   GO4.AnalysisStatusEditor = function(dom, stat) {
      JSROOT.BasePainter.call(this, dom);
      this.stat = stat;
      this.changes = [["dummy0", "init0"],["dummy1","init1"]];  // changes array stepwise, index 0 = no step, index = stepindex+1
      this.clearChanges();
      this.clearShowstates();
   }

   GO4.AnalysisStatusEditor.prototype = Object.create(JSROOT.BasePainter.prototype);

   GO4.AnalysisStatusEditor.prototype.markChanged = function(key, step) {
      // first avoid duplicate keys:
      for (let index = 0; index < this.changes[step].length; index++) {
         if (this.changes[step][index]== key) return;
      }
      this.changes[step].push(key);
      console.log("Mark changed :%s at step %d", key, step);

      this.selectDom().select(".buttonAnaChangeLabel").style('display', null); // show warning sign
   }

   // clear changed elements' list, make warning sign invisible
   GO4.AnalysisStatusEditor.prototype.clearChanges = function() {
      let numsteps = this.changes.length;
      for (let step = 0; step < numsteps ; step++)
         this.changes[step] = [];
      this.selectDom().select(".buttonAnaChangeLabel").style('display', 'none'); // hide warning sign
   }

   GO4.AnalysisStatusEditor.prototype.clearShowstates = function() {
   }

   //scan changed value list and return optionstring to be send to server
   GO4.AnalysisStatusEditor.prototype.evaluateChanges = function(optionstring) {
      let editor = this;
      let numsteps = this.changes.length;
      for (let step = 0; step < numsteps ; step++) {
         let len = this.changes[step].length;
         let stepoptions = "";
         for (let index = 0; index < len ; index++) {

            let key = this.changes[step][index];
            //console.log("Evaluate change key:%s", key);

            let theElement = editor.stat.fxStepArray.arr[step];
            // here mapping of key to editor field:
            if (key == "stepenabled") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fbProcessEnabled;
            } else if (key == "sourceenabled") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fbSourceEnabled;
            } else if (key == "storeenabled") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fbStoreEnabled;
            } else if (key == "sourcesel") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fiID;
            } else if (key == "sourcename") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fName;
            } else if (key == "sourcetag") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fxTagFile;
            } else if (key == "sourceport") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fiPort;
            } else if (key == "sourcetmout") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fiTimeout;
            } else if (key == "sourceretry") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fiRetryCnt;
            } else if (key == "sourcefirst") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fuStartEvent;
            } else if (key == "sourcelast") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fuStopEvent;
            } else if (key == "sourceskip") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxSourceType.fuEventInterval;
            } else if (key == "storesel") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxStoreType.fiID;
            } else if (key == "storename") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxStoreType.fName;
            } else if (key == "storesplit") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxStoreType.fiSplit;
            } else if (key == "storebuf") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxStoreType.fiBufsize;
            } else if (key == "storecomp") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxStoreType.fiCompression;
            } else if (key == "storeasf") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxStoreType.fiAutosavesize;
            } else if (key == "storeover") {
               stepoptions += "&" + key + "_" + step + "=" + theElement.fxStoreType.fbOverwrite;
            }
            // non step specific options are in step 0 options too:
            else if (key == "asfname") {
               stepoptions += "&" + key + "=" + editor.stat.fxAutoFileName;
            } else if (key == "asfenabled") {
               stepoptions += "&" + key + "=" + editor.stat.fbAutoSaveOn;
            } else if (key == "asftime") {
               stepoptions += "&" + key + "=" + editor.stat.fiAutoSaveInterval;
            } else if (key == "asfcomp") {
               stepoptions += "&" + key + "=" + editor.stat.fiAutoSaveCompression;
            } else if (key == "asfoverwrite") {
               stepoptions += "&" + key + "=" + editor.stat.fbAutoSaveOverwrite;
            } else if (key == "anaprefsname") {
               stepoptions += "&" + key + "=" + editor.stat.fxConfigFileName;
            } else {
               console.log("Warning: evaluateChanges found unknown key:%s", key);
            }

         }// for index

         optionstring+=stepoptions;
      } // for step
      console.log("Resulting option string:%s", optionstring);
      return optionstring;
   }

   GO4.AnalysisStatusEditor.prototype.refreshEditor = function() {
      let id = "#" + this.getDomId(),
          editor = this,
          stat = this.stat,
          dom = this.selectDom();

      ///////////// ANALYSIS STEPS:
      this.clearShowstates();
      let tabelement = $(id+" .steptabs");
      tabelement.tabs( "option", "disabled", [0, 1, 2, 3, 4, 5, 6, 7] );
      for(let j=0; j<8;++j){
         $(id +" .steptabs ul:first li:eq("+ j +")").hide(); // disable and hide all tabs
      }
      stat.fxStepArray.arr.forEach(function(element, index) {
         tabelement.tabs("enable",index);
         $(id +" .steptabs ul:first li:eq("+index+") a").text(element.fName);
         $(id +" .steptabs ul:first li:eq("+index+")").show(); // only show what is really there
         //console.log("refreshEditor for step name:"+ element.fName);
         tabelement.tabs("load",index); // all magic is in the on load event callback
      }); // for each

      /////////////////// AUTO SAVE FILE:
      dom.select(".anaASF_name").property("value", stat.fxAutoFileName);
      dom.select(".anaASF_enabled")
         .property('checked', stat.fbAutoSaveOn)
         .on("click", () =>  {
               this.markChanged("asfenabled",0);
               this.stat.fbAutoSaveOn = dom.select(".anaASF_enabled").property('checked');
             });

      dom.select(".anaASF_time")
          .property("value", stat.fiAutoSaveInterval)
          .on("change", () => {
             this.markChanged("asftime", 0);
             this.stat.fiAutoSaveInterval = dom.select(".anaASF_time").property("value");
         });
      dom.select(".anaASF_compression")
         .property("value", stat.fiAutoSaveCompression)
         .on("change", () => {
            this.markChanged("asfcomp", 0);
            this.stat.fiAutoSaveCompression = dom.select(".anaASF_compression").property("value");
         });

      dom.select(".anaASF_overwrite")
         .property('checked', stat.fbAutoSaveOverwrite)
         .on("click", () => {
                this.markChanged("asfoverwrite",0);
                this.stat.fbAutoSaveOverwrite = dom.select(".anaASF_overwrite").property('checked');
             });

      ////////////////// PREFS FILE:
      dom.select(".anaprefs_name").property("value", stat.fxConfigFileName);

      editor.clearChanges();
   }

   GO4.AnalysisStatusEditor.prototype.showStepEditor = function(tab, theElement, theIndex)
   {
      let id = "#" + this.getDomId(),
          editor = this;

      let sourcebox = tab.select(".step_box_source_enab"),
          storebox = tab.select(".step_box_store_enab"),
          step_source = tab.select(".step_source"),
          step_store = tab.select(".step_store"),
          sourcemore = tab.select(".step_source_expand");

      let sourceargs = tab.select(" .step_source_args");

      let storesplit = tab.select(" .step_store_split");
      let storebuf = tab.select(" .step_store_buf");
      let storecomp = tab.select(" .step_store_comp");
      let storetreeasf = tab.select(" .step_store_asf");
      let storeover = tab.select(" .step_store_overwrite");

    // here step control checkboxes and source/store visibility:
      if (theElement.fbProcessEnabled) {
         sourcebox.attr('disabled', null);
         storebox.attr('disabled', null);
         step_source.attr('disabled', theElement.fbSourceEnabled ? null : "true");
         step_store.style('display', theElement.fbStoreEnabled ? null : "none");
      } else {
         sourcebox.attr('disabled', "true");
         storebox.attr('disabled', "true");
         step_source.attr('disabled', "true");
         step_store.style('display', "none");
      }

      let show_tag = false, show_ports = false, show_args = false, show_more = sourcemore.property("checked");

      switch (theElement.fxSourceType.fiID) {
         case GO4.EvIOType.GO4EV_MBS_FILE:
            show_tag = show_args = show_more; break;
         case GO4.EvIOType.GO4EV_MBS_STREAM:
         case GO4.EvIOType.GO4EV_MBS_TRANSPORT:
         case GO4.EvIOType.GO4EV_MBS_EVENTSERVER:
         case GO4.EvIOType.GO4EV_MBS_REVSERV:
            show_ports = show_args = show_more; break;
         case GO4.EvIOType.GO4EV_USER:
            show_ports = show_more; break;
         case GO4.EvIOType.GO4EV_FILE:
         case GO4.EvIOType.GO4EV_MBS_RANDOM:
         default:
            // show no extra parameters
            break;
      };

      tab.select(".step_source_tagfile_args").style('display', show_tag ? null : 'none');
      tab.select(".step_source_port_args").style('display', show_ports ? null : 'none');
      tab.select(".step_source_number_args").style('display', show_args ? 'inline' : 'none');

      let enbale_store_pars = false, enbale_store_name = true;
      switch (theElement.fxStoreType.fiID) {
         case GO4.EvIOType.GO4EV_FILE:
            enbale_store_pars = true;
            break;
         case GO4.EvIOType.GO4EV_BACK:
            enbale_store_pars = true;
            enbale_store_name = false;
            break;
         case GO4.EvIOType.GO4EV_USER:
            break;
         default:
            break;
      };

      tab.select(".step_store_pars").selectAll("input").each(function() {
         d3.select(this).attr("disabled", enbale_store_pars ? null : "true");
      });

      tab.select(".step_store_name").attr("disabled", enbale_store_name ? null : "true");

      return;


      //console.log("show step editor with source id:"+theElement.fxSourceType.fiID);
      switch (theElement.fxSourceType.fiID) {
         case GO4.EvIOType.GO4EV_FILE:
         case GO4.EvIOType.GO4EV_MBS_RANDOM:
            sourceport.hide();
            sourceportlabel.hide();
            sourcetmout.hide();
            sourcetmoutlabel.hide();
            sourceretry.hide();
            sourceretrylabel.hide();
            sourcetag.hide();
            sourcetaglabel.hide();
            sourcefirst.hide();
            sourcefirstlabel.hide();
            sourcelast.hide();
            sourcelastlabel.hide()
            sourceskip.hide();
            sourceskiplabel.hide();
            sourceargs.hide();
            sourceargslabel.hide();
            break;
         case GO4.EvIOType.GO4EV_MBS_STREAM:
         case GO4.EvIOType.GO4EV_MBS_TRANSPORT:
         case GO4.EvIOType.GO4EV_MBS_EVENTSERVER:
         case GO4.EvIOType.GO4EV_MBS_REVSERV:
            if (showmore) {
               sourceport.show();
               sourceportlabel.show();
               sourcetmout.show();
               sourcetmoutlabel.show();
               sourceretry.show();
               sourceretrylabel.show();
               sourcefirst.show();
               sourcefirstlabel.show();
               sourcelast.show();
               sourcelastlabel.show();
               sourceskip.show();
               sourceskiplabel.show();
            } else {
               sourceport.hide();
               sourceportlabel.hide();
               sourcetmout.hide();
               sourcetmoutlabel.hide();
               sourceretry.hide();
               sourceretrylabel.hide();
               sourcefirst.hide();
               sourcefirstlabel.hide();
               sourcelast.hide();
               sourcelastlabel.hide();
               sourceskip.hide();
               sourceskiplabel.hide();
            }
            sourcetag.hide();
            sourcetaglabel.hide();
            sourceargs.hide();
            sourceargslabel.hide();
            break;
         case GO4.EvIOType.GO4EV_USER:
            if (showmore) {
               sourceport.show();
               sourceportlabel.show();
               sourcetmout.show();
               sourcetmoutlabel.show();
               sourceargs.show();
               sourceargslabel.show();
            } else {
               sourceport.hide();
               sourceportlabel.hide();
               sourcetmout.hide();
               sourcetmoutlabel.hide();
               sourceargs.hide();
               sourceargslabel.hide();
            }
            sourceretry.hide();
            sourceretrylabel.hide();
            sourcetag.hide();
            sourcetaglabel.hide();
            sourcefirst.hide();
            sourcefirstlabel.hide();
            sourcelast.hide();
            sourcelastlabel.hide();
            sourceskip.hide();
            sourceskiplabel.hide();
            break;
         default:
            console.log("showStepEditor WARNING: unknown event source id: " + theElement.fxSourceType.fiID);
         case GO4.EvIOType.GO4EV_MBS_FILE:
            if (showmore) {
               sourcetag.show();
               sourcetaglabel.show();
               sourcefirst.show();
               sourcefirstlabel.show();
               sourcelast.show();
               sourcelastlabel.show();
               sourceskip.show();
               sourceskiplabel.show();
            } else {
               sourcetag.hide();
               sourcetaglabel.hide();
               sourcefirst.hide();
               sourcefirstlabel.hide();
               sourcelast.hide();
               sourcelastlabel.hide();
               sourceskip.hide();
               sourceskiplabel.hide();
            }
            sourceport.hide();
            sourceportlabel.hide();
            sourcetmout.hide();
            sourcetmoutlabel.hide();
            sourceretry.hide();
            sourceretrylabel.hide();
            sourceargs.hide();
            sourceargslabel.hide();
            break;
      };

      storesplit.show();
      storebuf.show();
      storecomp.show();
      storetreeasf.show();
      storeover.show();
      //console.log("show step editor with store id:"+theElement.fxStoreType.fiID);
      switch (theElement.fxStoreType.fiID) {
         default:
            console.log("showStepEditor WARNING: unknown event store id: " + theElement.fxStoreType.fiID);
         case GO4.EvIOType.GO4EV_FILE:
            storecomp.spinner("enable");
            storetreeasf.spinner("enable");
            storeover.prop('disabled', false);
            break;
         case GO4.EvIOType.GO4EV_BACK:
            storecomp.spinner("disable");
            storetreeasf.spinner("disable");
            storeover.prop('disabled', true);
            break;
      };

      storesel.selectmenu("option", "width", storetable.width() * 0.8); // expand to table width
      storesel.selectmenu('refresh', true);

      pthis.css("padding", "5px");

      $(id + " .steptabs").tabs("refresh");
   }

   GO4.AnalysisStatusEditor.prototype.fillEditor = function()
   {
      let id = "#" + this.getDomId(),
          editor = this,
          dom = this.selectDom(),
          stat = this.stat;

      stat.fxStepArray.arr.forEach((step, indx) => {
         let tab = dom.select(".ana_step_tabs_body").select(`.go4_analysis_step_${indx}`);

         let theIndex = indx, pthis = tab, theElement = step;

         let step_source = tab.select(".step_source"),
             step_store = tab.select(".step_store"),
             enablebox = tab.select(".step_box_step_enab"),
             sourcebox = tab.select(".step_box_source_enab"),
             storebox = tab.select(".step_box_store_enab"),
             sourcesel = tab.select(".step_source_select"),
             sourcemore = tab.select(".step_source_expand"),
             sourcename = tab.select(".step_source_name"),
             sourcetag = tab.select(".step_source_tagfile"),
             sourceport = tab.select(".step_source_port"),
             sourcetmout = tab.select(".step_source_tmout"),
             sourceretry = tab.select(".step_source_retry"),
             sourcefirst = tab.select(".step_source_firstev"),
             sourcelast = tab.select(".step_source_lastev"),
             sourceskip = tab.select(".step_source_stepev"),
             storesel = tab.select(".step_store_select"),
             storename = tab.select(".step_store_name");

         let sourceargs = tab.select(" .step_source_args");

         let storesplit = tab.select(" .step_store_split");
         let storebuf = tab.select(" .step_store_buf");
         let storecomp = tab.select(" .step_store_comp");
         let storetreeasf = tab.select(" .step_store_asf");
         let storeover = tab.select(" .step_store_overwrite");

         enablebox.property('checked', theElement.fbProcessEnabled)
            .on("click", () => {
               this.markChanged("stepenabled", theIndex);
               theElement.fbProcessEnabled = enablebox.property('checked');
               this.showStepEditor(tab, theElement, theIndex);
            }); // clickfunction

         sourcebox.property('checked', theElement.fbSourceEnabled)
            .on("click", () => {
               this.markChanged("sourceenabled", theIndex);
               theElement.fbSourceEnabled = sourcebox.property('checked');
               this.showStepEditor(tab, theElement, theIndex);
            }); // clickfunction

         storebox.property('checked', theElement.fbStoreEnabled)
            .on("click", () => {
               this.markChanged("storeenabled", theIndex);
               theElement.fbStoreEnabled = storebox.property('checked');
               this.showStepEditor(tab, theElement, theIndex);
            }); // clickfunction

         //// EVENT SOURCE: /////////////////////////////////////////////////
         sourcesel.property("value", theElement.fxSourceType.fiID).on("change", () => {
            this.markChanged("sourcesel", theIndex);

            theElement.fxSourceType.fiID = parseInt(sourcesel.property("value"));

            this.showStepEditor(tab, theElement, theIndex);
         }); // source selectmenu change

         sourcemore.on("click", () => {
            this.showStepEditor(tab, theElement, theIndex);
         });

         sourcename.property("value", theElement.fxSourceType.fName)
                   .on("change", () => {
                        this.markChanged("sourcename", theIndex);
                        theElement.fxSourceType.fName = sourcename.property("value").trim();
                    });

         sourcetag.property("value", theElement.fxSourceType.fxTagFile || "")
                  .on("change", () => {
                     this.markChanged("sourcetag", theIndex);
                     theElement.fxSourceType.fxTagFile = sourcetag.property("value").trim();
                  });

         sourceport.property("value", theElement.fxSourceType.fiPort || 0)
                   .on("change", () => {
                      this.markChanged("sourceport", theIndex);
                      theElement.fxSourceType.fiPort = parseInt(sourceport.property("value"));
                   });

         sourcetmout.property("value", theElement.fxSourceType.fiTimeout || 100)
                    .on("change", () => {
                        this.markChanged("sourcetmout", theIndex);
                        theElement.fxSourceType.fiTimeout = parseInt(sourcetmout.property("value"));
                    });

         sourceretry.property("value", theElement.fxSourceType.fiRetryCnt || 0)
                    .on("change", () => {
                        this.markChanged("sourceretry", theIndex);
                        theElement.fxSourceType.fiRetryCnt = parseInt(sourceretry.property("value"));
                     });

         sourcefirst.property("value", theElement.fxSourceType.fuStartEvent || 0)
                    .on("change", () => {
                        this.markChanged("sourcefirst", theIndex);
                        theElement.fxSourceType.fuStartEvent = parseInt(sourcefirst.property("value"));
                     });

         sourcelast.property("value", theElement.fxSourceType.fuStopEvent || 0)
                    .on("change", () => {
                       this.markChanged("sourcelast", theIndex);
                       theElement.fxSourceType.fuStopEvent = parseInt(sourcelast.property("value"));
                     });

         sourceskip.property("value", theElement.fxSourceType.fuEventInterval || 0)
                   .on("change", () => {
                      this.markChanged("sourceskip", theIndex);
                      theElement.fxSourceType.fuEventInterval = parseInt(sourceskip.property("value"));
                   });

         storesel.property("value", theElement.fxStoreType.fiID).on("change", () => {
            this.markChanged("storesel", theIndex);
            theElement.fxStoreType.fiID = parseInt(storesel.property("value"));
            this.showStepEditor(tab, theElement, theIndex);
         });

         storename.property("value", theElement.fxStoreType.fName)
            .on("change", () => {
               this.markChanged("storename", theIndex);
               theElement.fxStoreType.fName = storename.property("value").trim();
            });

         this.showStepEditor(tab, theElement, theIndex); // handle all visibility issues here, also refresh tabs

         return;

         //console.log("on tab load finds store name: "+ theElement.fxStoreType.fName);

         storesplit.spinner({
            min: 0,
            max: 99,
            step: 1,
            stop: function(event, ui) {
               editor.markChanged("storesplit", theIndex);
               theElement.fxStoreType.fiSplit = this.value;
            }
         });

         storebuf.spinner({
            min: 4,
            max: 256,
            step: 1,
            stop: function(event, ui) {
               editor.markChanged("storebuf", theIndex);
               theElement.fxStoreType.fiBufsize = this.value * 1000;
            }
         });

         storecomp.spinner({
            min: 0,
            max: 9,
            step: 1,
            stop: function(event, ui) {
               editor.markChanged("storecomp", theIndex);
               theElement.fxStoreType.fiCompression = this.value;
            }
         });
         storetreeasf.spinner({
            min: 0,
            max: 99999,
            step: 100,
            stop: function(event, ui) {
               editor.markChanged("storeasf", theIndex);
               theElement.fxStoreType.fiAutosavesize = this.value;
            }
         });

         storeover.click(function() {
            editor.markChanged("storeover", theIndex);
            theElement.fxStoreType.fbOverwrite = this.checked;
         });

         ////////////////// here set event source values:


         // set event source selector and special fields:
         //console.log("load tab "+theIndex+" sees source id:"+theElement.fxSourceType.fiID);
         switch (theElement.fxSourceType.fiID) {
            case GO4.EvIOType.GO4EV_FILE:
               sourcesel.val(0);
               break;
            case GO4.EvIOType.GO4EV_MBS_STREAM:
               sourcesel.val(2);
               sourceport.val(theElement.fxSourceType.fiPort);
               sourcetmout.val(theElement.fxSourceType.fiTimeout);
               sourceretry.val(theElement.fxSourceType.fiRetryCnt);
               sourcetag.text(theElement.fxSourceType.fxTagFile);
               sourcefirst.val(theElement.fxSourceType.fuStartEvent);
               sourcelast.val(theElement.fxSourceType.fuStopEvent);
               sourceskip.val(theElement.fxSourceType.fuEventInterval);
               break;
            case GO4.EvIOType.GO4EV_MBS_TRANSPORT:
               sourcesel.val(3);
               sourceport.val(theElement.fxSourceType.fiPort);
               sourcetmout.val(theElement.fxSourceType.fiTimeout);
               sourceretry.val(theElement.fxSourceType.fiRetryCnt);
               sourcetag.text(theElement.fxSourceType.fxTagFile);
               sourcefirst.val(theElement.fxSourceType.fuStartEvent);
               sourcelast.val(theElement.fxSourceType.fuStopEvent);
               sourceskip.val(theElement.fxSourceType.fuEventInterval);
               break;
            case GO4.EvIOType.GO4EV_MBS_EVENTSERVER:
               sourcesel.val(4);
               sourceport.val(theElement.fxSourceType.fiPort);
               sourcetmout.val(theElement.fxSourceType.fiTimeout);
               sourceretry.val(theElement.fxSourceType.fiRetryCnt);
               sourcetag.text(theElement.fxSourceType.fxTagFile);
               sourcefirst.val(theElement.fxSourceType.fuStartEvent);
               sourcelast.val(theElement.fxSourceType.fuStopEvent);
               sourceskip.val(theElement.fxSourceType.fuEventInterval);
               break;
            case GO4.EvIOType.GO4EV_MBS_REVSERV:
               sourcesel.val(5);
               sourceport.val(theElement.fxSourceType.fiPort);
               sourcetmout.val(theElement.fxSourceType.fiTimeout);
               sourceretry.val(theElement.fxSourceType.fiRetryCnt);
               sourcetag.text(theElement.fxSourceType.fxTagFile);
               sourcefirst.val(theElement.fxSourceType.fuStartEvent);
               sourcelast.val(theElement.fxSourceType.fuStopEvent);
               sourceskip.val(theElement.fxSourceType.fuEventInterval);
               break;
            case GO4.EvIOType.GO4EV_MBS_RANDOM:
               sourcesel.val(6);
               break;
            case GO4.EvIOType.GO4EV_USER:
               sourcesel.val(7);
               sourceport.val(theElement.fxSourceType.fiPort);
               sourcetmout.val(theElement.fxSourceType.fiTimeout);
               sourceargs.text(theElement.fxSourceType.fxExpression);
               break;
            case GO4.EvIOType.GO4EV_MBS_FILE:
               sourcesel.val(1);
               sourcetag.text(theElement.fxSourceType.fxTagFile);
               sourcefirst.val(theElement.fxSourceType.fuStartEvent);
               sourcelast.val(theElement.fxSourceType.fuStopEvent);
               sourceskip.val(theElement.fxSourceType.fuEventInterval);
               break;
            default:
               console.log("WARNING: unknown event source id: " + theElement.fiID);
         };

         sourcesel.selectmenu('refresh', true);


         // event store properties:
         storesplit.val(theElement.fxStoreType.fiSplit);
         storebuf.val(theElement.fxStoreType.fiBufsize / 1000);
         storecomp.val(theElement.fxStoreType.fiCompression);
         storetreeasf.val(theElement.fxStoreType.fiAutosavesize);

         // set event store selector and special fields:
         //console.log("load tab "+theIndex+" sees store id:"+theElement.fxStoreType.fiID);
         switch (theElement.fxStoreType.fiID) {

            case GO4.EvIOType.GO4EV_BACK:
               storesel.val(1);
               break;
            case GO4.EvIOType.GO4EV_FILE:
               storeover.prop('checked', theElement.fxStoreType.fbOverwrite);
            default:
               storesel.val(0);
               break;
         };

         storesel.selectmenu('refresh', true);


      });// tabs loop

//////////////////////// END ANALYSIS STEP TABS

      dom.select(".buttonGetAnalysis")
         .style('background-image', "url(" + GO4.source_dir + "icons/right.png)")
         .on("click", () => {
               if (JSROOT.hpainter) JSROOT.hpainter.display(this.getItemName());
         });

      dom.select(".buttonSetAnalysis")
         .style('background-image', "url(" + GO4.source_dir + "icons/left.png)")
         .on("click", () => {
            let options = this.evaluateChanges(""); // complete option string from all changed elements
            console.log("submit analysis " + this.getItemName() + ", options=" + options);
            GO4.ExecuteMethod(this, "UpdateFromUrl", options).then(() => {
               console.log("setting analyis configuration done.");
               this.clearChanges();
               if (JSROOT.hpainter && (typeof JSROOT.hpainter.reload == 'function')) JSROOT.hpainter.reload();
            });
         });

      dom.select(".buttonAnaChangeLabel")
         .style('background-image', "url(" + GO4.source_dir + "icons/info1.png)");

      dom.select(".buttonSetStartAnalysis")
         .style('background-image', "url(" + GO4.source_dir + "icons/start.png)")
         .on("click", () => {
            let options = this.evaluateChanges(""); // complete option string from all changed elements
            options += "&start";
            console.log("submit and start analysis " + this.getItemName() + ", options=" + options);
            GO4.ExecuteMethod(this, "UpdateFromUrl", options).then(() => {
               console.log("submit and start analyis configuration done. ");
               this.clearChanges();
               if (JSROOT.hpainter && (typeof JSROOT.hpainter.reload == 'function')) JSROOT.hpainter.reload();
            });
         });

      dom.select(".buttonCloseAnalysis")
         .style('background-image', "url(" + GO4.source_dir + "icons/close.png)")
         .on("click", () => {
            let options = "&close";
            console.log("close analysis " + editor.getItemName() + ", options=" + options);
            GO4.ExecuteMethod(this, "UpdateFromUrl", options).then(() => {
               console.log("closing down analyis done. ");
            });
         });

      dom.select(".buttonSaveAnaASF")
         .style('background-image', "url(" + GO4.source_dir + "icons/filesave.png)");


      dom.select(".anaASF_form").on("submit", event => {
         event.preventDefault(); // do not send automatic request to server!
         let content = dom.select(".anaASF_name").property("value");
         content = content.trim();
         // before we write immediately, mark name as changed in setup:
         this.markChanged("asfname", 0);
         this.stat.fxAutoFileName = content;

         jsrp.createMenu(event, this)
             .then(menu => menu.runModal("Save auto save file",`<p tabindex="0">${content}</p>`, { btns: true, height: 120, width: 400 }))
             .then(elem => (elem ? GO4.ExecuteMethod(this, "UpdateFromUrl", "&saveasf=" + content) : null))
             .then(() => console.log("Writing autosave file done. "));
      });


      dom.select(".buttonSaveAnaConf")
         .style('background-image', "url(" + GO4.source_dir + "icons/filesave.png)")
         .on("click", event => {
            let content = dom.select(".anaprefs_name").property("value").trim(),
                requestmsg = "Really save analysis preferences: " + content;
            jsrp.createMenu(event, this)
                .then(menu => menu.runModal("Saving analysis preferences",`<p tabindex="0">${requestmsg}</p>`, { btns: true, height: 120, width: 400 }))
                .then(elem => {
                     if (!elem) return null;
                     this.markChanged("anaprefsname", 0);
                     this.stat.fxConfigFileName = content;
                     return GO4.ExecuteMethod(this, "UpdateFromUrl", "&saveprefs=" + content);
                 }).then(res => console.log(res !== null ? "Loading Saving preferences done. " : "Saving preferences  FAILED."));
         });

      dom.select(".buttonLoadAnaConf")
         .style('background-image', "url(" + GO4.source_dir + "icons/fileopen.png)")
         .on("click", event => {
            let content = dom.select(".anaprefs_name").property("value").trim(),
                requestmsg = "Really load analysis preferences: " + content;
            jsrp.createMenu(event, this)
                .then(menu => menu.runModal("Loading analysis preferences",`<p tabindex="0">${requestmsg}</p>`, { btns: true, height: 120, width: 400 }))
                .then(elem => (elem ? GO4.ExecuteMethod(this, "UpdateFromUrl", "&loadprefs=" + content) : null))
                .then(res => { if ((res!==null) && JSROOT.hpainter) JSROOT.hpainter.display(this.getItemName()); });
         });

      this.refreshEditor();
   }

   GO4.AnalysisStatusEditor.prototype.redrawObject = function(obj /*, opt */) {
      if (obj._typename != this.stat._typename) return false;
      this.stat = JSROOT.clone(obj);
      this.refreshEditor();
      return true;
   }

   GO4.drawGo4AnalysisStatus = function(domarg, stat) {
      let editor = new GO4.AnalysisStatusEditor(domarg, stat),
          dom = editor.selectDom(),
          h = dom.node().clientHeight,
          w = dom.node().clientWidth;

      if ((h < 10) && (w > 10)) dom.style("height", Math.round(w * 0.7)+"px");

      return JSROOT.httpRequest(GO4.source_dir + "html/analysiseditor.htm", "text").then(code => {
         dom.html(code);

         return JSROOT.httpRequest(GO4.source_dir + "html/stepeditor.htm", "text");

      }).then(step_code => {

         let head_html = "", step_html = "";
         stat.fxStepArray.arr.forEach((step, indx) => {
            head_html += `<button for="go4_analysis_step_${indx}">${step.fName}</button>`;
            step_html += `<div class="go4_analysis_step_${indx}" style="display:none">${step_code}</div>`;
         });

         dom.select(".ana_step_tabs_header").html(head_html);

         dom.select(".ana_step_tabs_body").html(step_html);

          // assign tabs buttons handlers
         dom.select('.ana_step_tabs_header').selectAll("button").on("click", function() {
            let btn = d3.select(this);
            dom.selectAll('.ana_step_tabs_body>div').each(function() {
               let tab = d3.select(this);
               tab.style('display', tab.classed(btn.attr("for")) ? null : "none");
            });
         });

         editor.fillEditor();

         // activate first step
         dom.select(".ana_step_tabs_body").select(".go4_analysis_step_0").style('display', null);

         editor.setTopPainter();
         return editor;
      });

   }

}); // factory function
