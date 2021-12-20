// $Id$

(function() {

   "use strict";

   if (typeof JSROOT != "object") {
      let e1 = new Error("go4.js requires JSROOT to be already loaded");
      e1.source = "go4.js";
      throw e1;
   }

  if (typeof GO4 == "object") {
      let e1 = new Error("GO4 already defined when loading go4.js");
      e1.source = "go4.js";
      throw e1;
   }

   globalThis.GO4 = { version: "6.1.4", id_counter: 1 };

   // use location to load all other scripts when required
   GO4.source_dir = function() {
      let scripts = document.getElementsByTagName('script');

      for (let n in scripts) {
         if (scripts[n]['type'] != 'text/javascript') continue;

         let src = scripts[n]['src'];
         if ((src == null) || (src.length == 0)) continue;

         let pos = src.indexOf("html/go4.js");
         if (pos<0) continue;
         if ((src.indexOf("JSRootCore") >= 0) || (src.indexOf("JSRoot.core") >= 0)) continue;
         console.log('Set GO4.source_dir to ' + src.substr(0, pos));
         return src.substr(0, pos);
      }
      return "";
   }();

   JSROOT.BasePainter.prototype.getDomId = function() {
      let elem = this.selectDom();
      if (elem.empty()) return "";
      let id = elem.attr("id");
      if (!id) {
         id = "go4_element_" + GO4.id_counter++;
         elem.attr("id", id);
      }
      return id;
   }

   if (typeof JSROOT.httpRequest == 'function')
      GO4.httpRequest = JSROOT.httpRequest;
   else
      GO4.httpRequest = function(url, kind, post_data) {
         return new Promise((resolveFunc,rejectFunc) => {
            let req = JSROOT.NewHttpRequest(url,kind, (res) => {
               if (res === null)
                  rejectFunc(Error(`Fail to request ${url}`));
               else
                  resolveFunc(res);
            });

            req.send(post_data || null);
         });
      }

   GO4.ExecuteMethod = function(item, method, options, callback) {
      let prefix = "";
      if (item.getItemName())
         prefix = item.getItemName() + "/"; // suppress / if item name is empty
      prefix += "exe.json\?method=";

      let fullcom = prefix + method + (options || "&"); // send any arguments otherwise ROOT refuse to process it

      GO4.httpRequest(fullcom, 'text')
         .then(() => callback(true))
         .catch(() => callback(false))
         .finally(() => console.log('Command is completed ' + prefix + method));
   }

   // ==================================================================================

   GO4.DrawAnalysisRatemeter = function(divid, itemname) {

      function CreateHTML() {
         let elem = d3.select('#'+divid);

         if (elem.size() == 0) return null;
         if (elem.select(".event_rate").size() > 0) return elem;

         let html = "<div style='padding-top:2px'>";
         html += "<img class='go4_logo' style='vertical-align:middle;margin-left:5px;margin-right:5px;' src='go4sys/icons/go4logorun4.gif' alt='logo'></img>";
         html += "<label class='event_source' style='border: 1px solid gray; font-size:large; vertical-align:middle; padding-left:3px; padding-right:3px;'>file.lmd</label> ";
         html += "<label class='event_rate' style='border: 1px solid gray; font-size:large; vertical-align:middle; background-color: grey'; padding-left:3px; padding-right:3px;>---</label> Ev/s ";
         html += "<label class='aver_rate' style='border: 1px solid gray; font-size:large; vertical-align:middle; padding-left:3px; padding-right:3px;'>---</label> Ev/s ";
         html += "<label class='run_time' style='border: 1px solid gray; font-size:large; vertical-align:middle; padding-left:3px; padding-right:3px;'>---</label> s ";
         html += "<label class='total_events' style='border: 1px solid gray; font-size:large; vertical-align:middle; padding-left:3px; padding-right:3px;'>---</label> Events ";
         html += "<label class='analysis_time' style='border: 1px solid gray; font-size:large; vertical-align:middle; padding-left:3px; padding-right:3px;'>time</label>";
         html += "</div>";

         elem.style('overflow','hidden')
             .style('padding-left','5px')
             .style('display', 'inline-block')
             .style('white-space', 'nowrap')
             .html(html);

         // use height of child element
         let brlayout = JSROOT.hpainter ? JSROOT.hpainter.brlayout : null,
             sz = $('#'+divid + " div").height() + 4; // use jquery to get height

         if (brlayout)
            brlayout.adjustSeparators(null, sz, true);
         return elem;
      }

      let xreq = false, was_running = null;

      function UpdateRatemeter() {
         if (xreq) return;

         let elem = CreateHTML();
         if (!elem) return;

         xreq = true;
         GO4.httpRequest(itemname+"/root.json.gz", 'object').then(res => {
            elem.select(".event_rate").style('background-color', res.fbRunning ? 'lightgreen' : 'red');
            if (was_running != res.fbRunning)
               elem.select(".go4_logo").attr("src", res.fbRunning ? 'go4sys/icons/go4logorun4.gif' : 'go4sys/icons/go4logo_t.png');

            was_running = res.fbRunning;

            elem.select(".event_source").text(res.fxEventSource == "" ? "<source>" : res.fxEventSource);
            elem.select(".event_rate").text(res.fdRate.toFixed(1));
            elem.select(".aver_rate").text((res.fdTime > 0 ? res.fuCurrentCount / res.fdTime : 0).toFixed(1));
            elem.select(".run_time").text(res.fdTime.toFixed(1));
            elem.select(".total_events").text(res.fuCurrentCount);
            elem.select(".analysis_time").text(res.fxDateString == "" ? "<datime>" : res.fxDateString);
         }).catch(() => {
            elem.select(".event_rate").style('background-color','grey');
         }).finally(() => {
            xreq = false;
         });
      }

      CreateHTML();

      setInterval(UpdateRatemeter, 2000);
   }


   GO4.MakeMsgListRequest = function(hitem, item) {
      let arg = "&max=2000";
      if ('last-id' in item) arg+= "&id="+item['last-id'];
      return 'exe.json.gz?method=Select' + arg;
   }

   GO4.AfterMsgListRequest = function(hitem, item, obj) {
      if (!item) return;

      if (!obj) {
         delete item['last-id'];
         return;
      }
      // ignore all other classes
      if (obj._typename != 'TList') return;

      obj._typename = "TGo4MsgList";

      if (obj.arr.length>0) {
         let duplicate = (('last-id' in item) && (item['last-id'] == obj.arr[0].fString));

         item['last-id'] = obj.arr[0].fString;

         // workaround for snapshot, it sends always same messages many times
         if (duplicate) obj.arr.length = 1;

         // add clear function for item
         if (!('clear' in item))
            item['clear'] = function() { delete this['last-id']; }
      }
   }


   GO4.MsgListPainter = function(divid, lst) {
      JSROOT.BasePainter.call(this, divid);
      if (this.SetDivId) this.SetDivId(divid); // old
      this.lst = lst;
      return this;
   }

   GO4.MsgListPainter.prototype = Object.create( JSROOT.BasePainter.prototype );

   GO4.MsgListPainter.prototype.redrawObject = function(obj) {
      // if (!obj._typename != 'TList') return false;
      this.lst = obj;
      this.drawList();
      return true;
   }

   GO4.MsgListPainter.prototype.drawList = function() {
      if (!this.lst) return;

      let frame = this.selectDom();

      let main = frame.select("div");
      if (main.empty())
         main = frame.append("div")
                     .style('max-width','100%')
                     .style('max-height','100%')
                     .style('overflow','auto');

      let old = main.selectAll("pre");
      let newsize = old.size() + this.lst.arr.length - 1;

      // in the browser keep maximum 2000 entries
      if (newsize > 2000)
         old.select(function(d,i) { return i < newsize - 2000 ? this : null; }).remove();

      for (let i = this.lst.arr.length-1; i > 0; i--)
         main.append("pre").style('margin','3px').html(this.lst.arr[i].fString);
   }

   GO4.DrawMsgList = function(divid, lst, opt) {

      let painter = new GO4.MsgListPainter(divid, lst);

      painter.drawList();

      painter.setTopPainter();
      return Promise.resolve(painter);
   }

   GO4.drawAnalysisTerminal = function(hpainter, itemname) {
      let url = hpainter.getOnlineItemUrl(itemname),
          mdi = hpainter.getDisplay(),
          frame = mdi ? mdi.findFrame(itemname, true) : null;

      if (!url || !frame) return null;

      let divid = d3.select(frame).attr('id');

      let h = $("#"+divid).height(), w = $("#"+divid).width();
      if ((h < 10) && (w > 10)) $("#"+divid).height(w*0.7);

      let player = new JSROOT.BasePainter(divid);
      player.url = url;
      player.hpainter = hpainter;
      player.itemname = itemname;
      player.draw_ready = true;
      player.needscroll = false;

      player.logReady = function(p) {
         if (p) this.log_painter = p;
         if(this.needscroll) {
            this.clickScroll();
            this.needscroll = false;
         }
         this.draw_ready = true;
      }

      player.cleanup = function() {
         if (this.log_painter) {
            this.log_painter.cleanup();
            delete this.log_painter;
         }
         if (this.interval) {
            clearInterval(this.interval);
            delete this.interval;
         }
         JSROOT.BasePainter.prototype.cleanup.call(this);
      }

      player.processTimer = function() {
         let subid = "anaterm_output_container";
         if ($("#" + subid).length == 0) {
            // detect if drawing disappear
            return this.cleanup();
         }
         if (!this.draw_ready) return;

         let msgitem = this.itemname.replace("Control/Terminal", "Status/Log");

         this.draw_ready = false;

         if (this.log_painter)
            this.hpainter.display(msgitem, "update:divid:" + subid).then(() => this.logReady());
         else
            this.hpainter.display(msgitem, "divid:" + subid).then(p => this.logReady(p));
      }

      player.ClickCommand = function(kind) {
         let command = this.itemname.replace("Terminal", "CmdExecute");
         this.hpainter.executeCommand(command, null, kind). then(() => { this.needscroll = true; });
      }

      player.ClickClear = function() {
         document.getElementById("anaterm_output_container").firstChild.innerHTML = "";
      }

      player.clickScroll = function() {
         //  inner frame created by hpainter has the scrollbars, i.e. first child
         let disp = $("#anaterm_output_container").children(":first");
         disp.scrollTop(disp[0].scrollHeight - disp.height());
      }


      player.fillDisplay = function(id) {
         this.setTopPainter();
         this.interval = setInterval(() => this.processTimer(), 2000);

         id = "#" + id; // to use in jQuery

         $(id + " .go4_clearterm")
            .button({ text: false, icons: { primary: "ui-icon-blank MyTermButtonStyle" } })
            .click(this.ClickClear.bind(this))
            .children(":first") // select first button element, used for images
            .css('background-image', "url(" + GO4.source_dir + "icons/clear.png)");

         $(id + " .go4_endterm")
            .button({ text: false, icons: { primary: "ui-icon-blank MyTermButtonStyle" } })
            .click(this.clickScroll.bind(this))
            .children(":first") // select first button element, used for images
            .css('background-image', "url(" + GO4.source_dir + "icons/shiftdown.png)");

         $(id + " .go4_printhistos")
            .button({ text: false, icons: { primary: "ui-icon-blank MyTermButtonStyle" } })
            .click(this.ClickCommand.bind(this, "@PrintHistograms()"))
            .children(":first") // select first button element, used for images
            .css('background-image', "url(" + GO4.source_dir + "icons/hislist.png)");

         $(id + " .go4_printcond")
            .button({ text: false, icons: { primary: "ui-icon-blank MyTermButtonStyle" } })
            .click(this.ClickCommand.bind(this, "@PrintConditions()"))
            .children(":first") // select first button element, used for images
            .css('background-image', "url(" + GO4.source_dir + "icons/condlist.png)");

         let pthis = this;

         $("#go4_anaterm_cmd_form").submit(
            function(event) {
               let command = pthis.itemname.replace("Terminal", "CmdExecute");
               let cmdpar = document.getElementById("go4_anaterm_command").value;
               console.log("submit command - " + cmdpar);
               pthis.hpainter.executeCommand(command, null, cmdpar).then(() => { pthis.needscroll = true; });
               event.preventDefault();
            });

         $(id + " .go4_executescript")
            .button({ text: false, icons: { primary: "ui-icon-blank MyTermButtonStyle" } })
            .children(":first") // select first button element, used for images
            .css('background-image', "url(" + GO4.source_dir + "icons/macro_t.png)");
      }

      player.checkResize = function() {}

      $("#"+divid).load(GO4.source_dir + "html/terminal.htm", "", () => player.fillDisplay(divid));

      return player;
   }

   const op_LineColor   = 5,
         op_LineStyle   = 6,
         op_LineWidth   = 7,
         op_FillColor   = 8,
         op_FillStyle   = 9,
         op_HisStats    = 24,
         op_HisTitle    = 25,
         op_HisStatsOpt = 85,
         op_HisStatsFit = 86,
         PictureIndex   = -1;

   function getOptValue(pic, indx, typ) {
      if (!pic || !pic.fxOptIndex) return null;
      let srch = indx + 1 + (typ << 16);
      for (let k = 0; k < pic.fxOptIndex.length; ++k)
         if (pic.fxOptIndex[k] == srch)
            return pic.fxOptValue[k];
      return null;
   }

   function drawPictureObjects(divid, pic, k) {
      if (!divid || !pic.fxNames)
         return Promise.resolve(false);

      let arr = pic.fxNames ? pic.fxNames.arr : null;
      if (!arr || (k >= arr.length))
         return Promise.resolve(false);

      let n = pic.fxNames.arr[k], itemname = "", isth2 = false;

      JSROOT.hpainter.forEachItem(item => {
         if (item._name == n.fString) {
            itemname = JSROOT.hpainter.itemFullName(item);
            if (item._kind && (item._kind.indexOf("ROOT.TH2") == 0)) isth2 = true;
         }
      });

      if (!itemname) {
         console.log('not found object with name', n.fString);
         return drawPictureObjects(divid, pic, k+1);
      }

      // console.log('Want to display item', itemname, 'on', divid);

      let opt = isth2 ? "col" : "";
      if (k > 0) opt += "same";

      return JSROOT.hpainter.display(itemname, opt + "divid:" + divid).then(painter => {
         if (!painter) return;
         let need_redraw = false;

         if (painter.lineatt) {
            let lcol = getOptValue(pic, k, op_LineColor),
                lwidth = getOptValue(pic, k, op_LineWidth),
                lstyle = getOptValue(pic, k, op_LineStyle);
            if ((lcol !== null) && (lwidth !== null) && (lstyle !== null)) {
               painter.lineatt.change(painter.getColor(lcol), lwidth, lstyle);
               need_redraw = true;
            }
         }

         if (painter.fillatt) {
            let fcol = getOptValue(pic, k, op_FillColor),
                fstyle = getOptValue(pic, k, op_FillStyle);

            if ((fcol !== null) && (fstyle !== null)) {
               painter.fillatt.change(fcol, fstyle, painter.getCanvSvg());
               need_redraw = true;
            }
         }

         if (typeof painter.getHisto == 'function' && painter.createHistDrawAttributes && painter.isMainPainter()) {
            const kNoStats = JSROOT.BIT(9),
                  kNoTitle = JSROOT.BIT(17);

            let histo = painter.getHisto(),
                stat = painter.findStat(),
                istitle = getOptValue(pic, PictureIndex, op_HisTitle),
                isstats = getOptValue(pic, PictureIndex, op_HisStats),
                statsopt = getOptValue(pic, PictureIndex, op_HisStatsOpt),
                fitopt = getOptValue(pic, PictureIndex, op_HisStatsFit);

            if ((istitle !== null) && (!istitle != histo.TestBit(kNoTitle))) {
               histo.InvertBit(kNoTitle); need_redraw = true;
            }
            if ((isstats !== null) && (!isstats != histo.TestBit(kNoStats))) {
               histo.InvertBit(kNoStats); need_redraw = true;
            }
            if (stat && ((statsopt !== null) || (fitopt !== null))) {
               if (statsopt !== null) stat.fOptStat = statsopt;
               if (fitopt !== null) stat.fOptFit = fitopt;
               let pp = painter.getPadPainter(),
                   statpainter = pp ? pp.findPainterFor(stat) : null;
               if (statpainter) statpainter.redraw();
            }
         }

         if (need_redraw) return painter.redraw();

      }).then(() => drawPictureObjects(divid, pic, k+1));
   }

   function drawSubPictures(pad_painter, pic, nsub) {
      let arr = pic && pic.fxSubPictures ? pic.fxSubPictures.arr : null;
      if (!arr || nsub >= arr.length)
         return Promise.resolve(pad_painter);

      let subpic = pic.fxSubPictures.arr[nsub];

      let subpad_painter = pad_painter.getSubPadPainter(1 + subpic.fiPosY*pic.fiNDivX + subpic.fiPosX);

      return drawPicture(subpad_painter, subpic).then(() => drawSubPictures(pad_painter, pic, nsub+1));
   }

   function drawPicture(pad_painter, pic) {
      if (!pad_painter)
         return Promise.resolve(false);

      let need_divide = pic.fiNDivX * pic.fiNDivY > 1;

      if (need_divide && !pad_painter.divide) {
         console.log('JSROOT version without TPadPainter.divide');
         return Promise.resolve(false);
      }

      let prev_name = pad_painter.selectCurrentPad(pad_painter.this_pad_name);

      if (need_divide)
         return pad_painter.divide(pic.fiNDivX, pic.fiNDivY).then(() => drawSubPictures(pad_painter, pic, 0)).then(() => {
            pad_painter.selectCurrentPad(prev_name);
            return pad_painter;
         });

      let divid = pad_painter.selectDom().attr('id');
      if (!divid) {
         divid = "go4picture_div_" + GO4.id_counter++;
         pad_painter.selectDom().attr('id', divid);
         console.error('Drawing must be done on element with id, force ', divid);
      }

      return drawPictureObjects(divid, pic, 0).then(() => {
         pad_painter.selectCurrentPad(prev_name);
         return pad_painter;
      });
   }

   GO4.drawGo4Picture = function(dom, pic) {
      if (!JSROOT.hpainter) return null;

      let painter = new JSROOT.ObjectPainter(dom, pic);

      return JSROOT.require('gpad').then(() => JSROOT.Painter.ensureTCanvas(painter, false)).then(() => {
         let pad_painter = painter.getPadPainter();

         painter.removeFromPadPrimitives();

         return drawPicture(pad_painter, pic);
      }).then(() => painter); // return dummy painter
   }

   // ==============================================================================

   let canvsrc = GO4.source_dir + 'html/go4canvas.js;';
   let jsrp = JSROOT.Painter;

   jsrp.addDrawFunc({ name: "TGo4WinCond",  script: canvsrc + GO4.source_dir + 'html/condition.js', func: 'GO4.drawGo4Cond', opt: ";editor" });
   jsrp.addDrawFunc({ name: "TGo4PolyCond", script: canvsrc + GO4.source_dir + 'html/condition.js', func: 'GO4.drawGo4Cond', opt: ";editor" });
   jsrp.addDrawFunc({ name: "TGo4ShapedCond", script: canvsrc + GO4.source_dir + 'html/condition.js', func: 'GO4.drawGo4Cond', opt: ";editor" });
   jsrp.addDrawFunc({ name: "TGo4CondArray", script: canvsrc + GO4.source_dir + 'html/condition.js', func: 'GO4.drawCondArray', opt: ";editor" });
   jsrp.addDrawFunc({ name: "TGo4Marker", script: canvsrc, func: 'GO4.drawGo4Marker' });

   jsrp.addDrawFunc({ name: "TGo4AnalysisWebStatus", script: GO4.source_dir + 'html/analysiseditor.js', func: 'GO4.drawGo4AnalysisStatus', opt: "editor" });

   jsrp.addDrawFunc({ name: "TGo4MsgList", func: GO4.DrawMsgList });
   jsrp.addDrawFunc({ name: "TGo4Picture", func: GO4.drawGo4Picture, icon: GO4.source_dir + "icons/picture.png" });

   jsrp.addDrawFunc({ name: "TGo4MbsEvent", noinspect: true });
   jsrp.addDrawFunc({ name: "TGo4EventElement", noinspect: true });

})(); // factory
