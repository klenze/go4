// $Id$

const go4Script = document.currentScript;

JSROOT.define(["painter"], jsrp => {

   "use strict";

   const GO4 = { version: "6.2.99", id_counter: 1, source_dir: "" };

   if (go4Script && (typeof go4Script.src == "string")) {
      const pos = go4Script.src.indexOf("html/go4.js");
      if (pos >= 0) {
         GO4.source_dir = go4Script.src.substr(0, pos);
         console.log(`Set GO4.source_dir to ${GO4.source_dir}, ${GO4.version}`);
      }
   }

   /** @summary Exactute method for selected painter object
     * @return {Promise} when done */
   GO4.ExecuteMethod = function(painter, method, options) {
      let prefix = "";
      if (painter.getItemName())
         prefix = painter.getItemName() + "/"; // suppress / if item name is empty

      let fullcom = prefix + "exe.json?method=" + method + (options || "&"); // send any arguments otherwise ROOT refuse to process it

      return JSROOT.httpRequest(fullcom, 'text');
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
             sz = elem.node().clientHeight + 12;

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
         JSROOT.httpRequest(itemname + "/root.json.gz", 'object').then(res => {
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

   class MsgListPainter extends JSROOT.BasePainter {

      constructor(dom, lst) {
         super(dom);
         this.lst = lst;
      }

      redrawObject(obj) {
         // if (!obj._typename != 'TList') return false;
         this.lst = obj;
         this.drawList();
         return true;
      }

      drawList() {
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
   }

   GO4.DrawMsgList = function(dom, lst) {

      let painter = new MsgListPainter(dom, lst);

      painter.drawList();

      painter.setTopPainter();
      return Promise.resolve(painter);
   }

   class AnalysisTerminalPainter extends JSROOT.BasePainter {
      constructor(frame, hpainter, itemname, url) {
         super(frame);
         this.hpainter = hpainter;
         this.itemname = itemname;
         this.url = url;
         this.draw_ready = true;
         this.needscroll = false;
      }

      logReady(p) {
         if (p) this.log_painter = p;
         if(this.needscroll) {
            this.clickScroll(true);
            this.needscroll = false;
         }
         this.draw_ready = true;
      }

      checkResize() {}

      cleanup(arg) {
         if (this.log_painter) {
            this.log_painter.cleanup();
            delete this.log_painter;
         }
         if (this.interval) {
            clearInterval(this.interval);
            delete this.interval;
         }
         super.cleanup(arg);
      }

      processTimer() {
         let subid = "anaterm_output_container";
         // detect if drawing disappear
         if (d3.select("#" + subid).empty())
            return this.cleanup();

         if (!this.draw_ready) return;

         let msgitem = this.itemname.replace("Control/Terminal", "Status/Log");

         this.draw_ready = false;

         if (this.log_painter)
            this.hpainter.display(msgitem, "update:divid:" + subid).then(() => this.logReady());
         else
            this.hpainter.display(msgitem, "divid:" + subid).then(p => this.logReady(p));
      }

      clickCommand(kind) {
         let command = this.itemname.replace("Terminal", "CmdExecute");
         this.hpainter.executeCommand(command, null, kind). then(() => { this.needscroll = true; });
      }

      clickClear() {
         d3.select("#anaterm_output_container").html("");
      }

      clickScroll(last) {
         //  inner frame created by hpainter has the scrollbars, i.e. first child
         let nodes = d3.select("#anaterm_output_container").selectAll("pre").nodes();
         if (nodes) nodes[last ? nodes.length-1 : 0].scrollIntoView();
      }

      fillDisplay() {
         this.setTopPainter();
         this.interval = setInterval(() => this.processTimer(), 2000);
         let dom = this.selectDom();

         dom.select(".go4_clearterm")
            .on("click", () => this.clickClear())
            .style('background-image', "url(" + GO4.source_dir + "icons/clear.png)");

         dom.select(".go4_startterm")
            .on("click", () => this.clickScroll(false))
            .style('background-image', "url(" + GO4.source_dir + "icons/shiftup.png)");

         dom.select(".go4_endterm")
            .on("click", () => this.clickScroll(true))
            .style('background-image', "url(" + GO4.source_dir + "icons/shiftdown.png)");

         dom.select(".go4_printhistos")
            .on("click", () => this.clickCommand("@PrintHistograms()"))
            .style('background-image', "url(" + GO4.source_dir + "icons/hislist.png)");

         dom.select(".go4_printcond")
            .on("click", () => this.clickCommand("@PrintConditions()"))
            .style('background-image', "url(" + GO4.source_dir + "icons/condlist.png)");

         dom.select(".go4_executescript")
            .style('background-image', "url(" + GO4.source_dir + "icons/macro_t.png)");

         dom.select(".go4_anaterm_cmd_form").on("submit", event => {
            event.preventDefault();
            let command = this.itemname.replace("Terminal", "CmdExecute");
            let cmdpar = document.getElementById("go4_anaterm_command").value;
            console.log("submit command - " + cmdpar);
            this.hpainter.executeCommand(command, null, cmdpar).then(() => { this.needscroll = true; });
         });

      }
   }

   GO4.drawAnalysisTerminal = function(hpainter, itemname) {
      let url = hpainter.getOnlineItemUrl(itemname),
          mdi = hpainter.getDisplay(),
          frame = mdi ? mdi.findFrame(itemname, true) : null;

      if (!url || !frame) return null;

      let elem = d3.select(frame),
          h = frame.clientHeight,
          w = frame.clientWidth;
      if ((h < 10) && (w > 10)) elem.style("height", Math.round(w*0.7)+"px");

      let player = new AnalysisTerminalPainter(frame, hpainter, itemname, url);

      return JSROOT.httpRequest(GO4.source_dir + "html/terminal.htm", "text")
                   .then(code => {
                     elem.html(code);
                     player.fillDisplay();
                     return player;
                   });
   }

   const op_LineColor   = 5,
         op_LineStyle   = 6,
         op_LineWidth   = 7,
         op_FillColor   = 8,
         op_FillStyle   = 9,
         op_HisStatsX1  = 19,
         op_HisStatsY1  = 20,
         op_HisStatsX2  = 21,
         op_HisStatsY2  = 22,
         op_HisStats    = 24,
         op_HisTitle    = 25,
         op_HisStatsOpt = 85,
         op_HisStatsFit = 86,
         op_Draw        = 0x4001,

         PictureIndex   = -1;

   function getOptValue(pic, indx, typ) {
      if (!pic || !pic.fxOptIndex) return null;
      let srch = indx + 1 + (typ << 16);
      for (let k = 0; k < pic.fxOptIndex.length; ++k)
         if (pic.fxOptIndex[k] == srch)
            return pic.fxOptValue[k];
      return null;
   }

   function getOptDouble(pic, indx, typ) {
      let ivalue = getOptValue(pic, indx, typ);
      if (ivalue === null) return null;
      const buffer = new ArrayBuffer(16),
            view = new DataView(buffer),
            big = BigInt(ivalue); // to be sure that BigInt is used
      view.setUint32(0, Number(big >> 32n));
      view.setUint32(4, Number(big % (2n ** 32n)));
      return view.getFloat64(0);
   }

   function drawSpecialObjects(divid, pic, k) {
      if (!pic.fxSpecialObjects || (k >= pic.fxSpecialObjects.arr.length))
         return Promise.resolve(false);

      return JSROOT.draw(divid, pic.fxSpecialObjects.arr[k], pic.fxSpecialObjects.opt[k]).then(() => drawSpecialObjects(divid, pic, k+1));
   }

   function drawPictureObjects(divid, pic, k) {
      if (!divid || !pic.fxNames)
         return Promise.resolve(false);

      let arr = pic.fxNames ? pic.fxNames.arr : null;
      if (!arr || (k >= arr.length))
         return drawSpecialObjects(divid, pic, 0);

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

      let opt = isth2 ? "col" : "",
          iopt = getOptValue(pic, k, op_Draw);
      if ((iopt !== null) && pic.fxOptObjects)
         opt = pic.fxOptObjects.arr[iopt].fString;
      if (k > 0) opt += " same";

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
                sx1 = getOptDouble(pic, PictureIndex, op_HisStatsX1),
                sy1 = getOptDouble(pic, PictureIndex, op_HisStatsY1),
                sx2 = getOptDouble(pic, PictureIndex, op_HisStatsX2),
                sy2 = getOptDouble(pic, PictureIndex, op_HisStatsY2),
                has_stats_pos = (sx1 !== null) && (sy1 !== null) && (sx2!==null) && (sy2!==null),
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
            if (stat && ((statsopt !== null) || (fitopt !== null) || has_stats_pos)) {
               if (statsopt !== null) stat.fOptStat = statsopt;
               if (fitopt !== null) stat.fOptFit = fitopt;
               let pp = painter.getPadPainter(),
                   statpainter = pp ? pp.findPainterFor(stat) : null;
               if (has_stats_pos) {
                  stat.fX1NDC = sx1;
                  stat.fY1NDC = sy1;
                  stat.fX2NDC = sx2;
                  stat.fY2NDC = sy2;
               }
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

      return JSROOT.require('gpad').then(() => jsrp.ensureTCanvas(painter, false)).then(() => {
         let pad_painter = painter.getPadPainter();

         painter.removeFromPadPrimitives();

         return drawPicture(pad_painter, pic);
      }).then(() => painter); // return dummy painter
   }

   // ==============================================================================

   let canvsrc = GO4.source_dir + 'html/go4canvas.js;';

   jsrp.addDrawFunc({ name: "TGo4WinCond",  script: canvsrc, func: 'GO4.drawGo4Cond', opt: ";editor" });
   jsrp.addDrawFunc({ name: "TGo4PolyCond", script: canvsrc, func: 'GO4.drawGo4Cond', opt: ";editor" });
   jsrp.addDrawFunc({ name: "TGo4ShapedCond", script: canvsrc, func: 'GO4.drawGo4Cond', opt: ";editor" });
   jsrp.addDrawFunc({ name: "TGo4CondArray", script: canvsrc, func: 'GO4.drawCondArray', opt: ";editor" });
   jsrp.addDrawFunc({ name: "TGo4Marker", script: canvsrc, func: 'GO4.drawGo4Marker' });

   jsrp.addDrawFunc({ name: "TGo4AnalysisWebStatus", script: GO4.source_dir + 'html/analysiseditor.js', func: 'GO4.drawGo4AnalysisStatus', opt: "editor" });

   jsrp.addDrawFunc({ name: "TGo4MsgList", func: GO4.DrawMsgList });
   jsrp.addDrawFunc({ name: "TGo4Picture", func: GO4.drawGo4Picture, icon: GO4.source_dir + "icons/picture.png" });

   jsrp.addDrawFunc({ name: "TGo4MbsEvent", noinspect: true });
   jsrp.addDrawFunc({ name: "TGo4EventElement", noinspect: true });

   globalThis.GO4 = GO4;

   return GO4;
});
