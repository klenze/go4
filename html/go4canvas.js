(function( factory ) {

   // this is code for web canvas to support Go4 classes like
   // TGo4Marker or TGo4Condition in the go4 gui
   // it is slightly different to go4.js which is dedicated with usage of THttpServer

   if (typeof JSROOT != "object") {
      var e1 = new Error("go4canvas.js requires JSROOT to be already loaded");
      e1.source = "go4canvas.js";
      throw e1;
   }

   var myGO4 = { version: "5.3.x", web_canvas: true };

   factory(JSROOT, (typeof GO4 != 'undefined') ? GO4 : myGO4);

} (function(JSROOT, GO4) {

   "use strict";

   GO4.MarkerPainter = function(marker) {
      JSROOT.TObjectPainter.call(this, marker);
      this.pave = null; // drawing of stat
   }

   GO4.MarkerPainter.prototype = Object.create(JSROOT.TObjectPainter.prototype);

   GO4.MarkerPainter.prototype.drawMarker = function(interactive) {
      if (interactive && this.draw_g) {
         this.draw_g.selectAll('*').remove();
      } else {
         this.CreateG(); // can draw in complete pad
      }

      var marker = this.GetObject();

      this.createAttMarker({ attr: marker });

      this.grx = this.AxisToSvg("x", marker.fX);
      this.gry = this.AxisToSvg("y", marker.fY);

      var path = this.markeratt.create(this.grx, this.gry);

      if (path)
          this.draw_g.append("svg:path")
             .attr("d", path)
             .call(this.markeratt.func);
   }

   GO4.MarkerPainter.prototype.fillLabels = function(marker) {
      var lbls = [];

      var main = this.main_painter(), hint = null;
      if (main && typeof main.ProcessTooltip == 'function')
         hint = main.ProcessTooltip({ enabled: false, x: this.grx - this.frame_x(), y: this.gry - this.frame_y() });

      lbls.push(marker.fxName + ((hint && hint.name) ? (" : " + hint.name) : ""));

      if (marker.fbXDraw)
          lbls.push("X = " + JSROOT.FFormat(marker.fX, "6.4g"));

      if (marker.fbYDraw)
         lbls.push("Y = " + JSROOT.FFormat(marker.fY, "6.4g"));

      if (hint && hint.user_info) {
         if (marker.fbXbinDraw) {
            var bin = "<undef>";
            if (hint.user_info.binx !== undefined) bin = hint.user_info.binx; else
            if (hint.user_info.bin !== undefined) bin = hint.user_info.bin;
            lbls.push("Xbin = " + bin);
         }

         if (marker.fbYbinDraw) {
            lbls.push("Ybin = " + ((hint.user_info.biny !== undefined) ? hint.user_info.biny : "<undef>"));
         }

         if (marker.fbContDraw)
            lbls.push("Cont = " + hint.user_info.cont);
      }

      return lbls;
   }

   GO4.MarkerPainter.prototype.drawLabel = function() {

      var marker = this.GetObject();

      if (!marker.fbHasLabel) return;

      var pave_painter = this.FindPainterFor(this.pave);

      if (!pave_painter) {
         this.pave = JSROOT.Create("TPaveStats");
         this.pave.fName = "stats_" + marker.fName;

         var px = this.grx / this.pad_width() + 0.02,
             py = this.gry / this.pad_height() - 0.02;
         JSROOT.extend(this.pave, { fX1NDC: px, fY1NDC: py - 0.15, fX2NDC: px + 0.2, fY2NDC: py, fBorderSize: 1, fFillColor: 0, fFillStyle: 1001 });

         var st = JSROOT.gStyle;
         JSROOT.extend(this.pave, { fFillColor: st.fStatColor, fFillStyle: st.fStatStyle, fTextAngle: 0, fTextSize: st.fStatFontSize,
                                    fTextAlign: 12, fTextColor: st.fStatTextColor, fTextFont: st.fStatFont});
      } else {
         this.pave.Clear();
      }

      var lbls = this.fillLabels(marker);
      for (var k=0;k<lbls.length;++k)
         this.pave.AddText(lbls[k]);

      if (pave_painter)
         pave_painter.Redraw();
      else
         JSROOT.draw(this.divid, this.pave, "", function(p) {
           if (p) p.$secondary = true
         });
   }

   GO4.MarkerPainter.prototype.RedrawObject = function(obj) {
      if (this.UpdateObject(obj))
         this.Redraw(); // no need to redraw complete pad
   }

   GO4.MarkerPainter.prototype.Cleanup = function(arg) {
      if (this.pave) {
         var pp = this.FindPainterFor(this.pave);
         if (pp) pp.DeleteThis();
         delete this.pave;
      }

      JSROOT.TObjectPainter.prototype.Cleanup.call(this, arg);
   }

   GO4.MarkerPainter.prototype.Redraw = function() {
      this.drawMarker();
      this.drawLabel();
   }

   GO4.MarkerPainter.prototype.ProcessTooltip = function(pnt) {
      var hint = this.ExtractTooltip(pnt);
      if (!pnt || !pnt.disabled) this.ShowTooltip(hint);
      return hint;
   }

   GO4.MarkerPainter.prototype.FillContextMenu = function(menu) {
      var marker = this.GetObject();
      menu.add("header:"+ marker._typename + "::" + marker.fxName);
      function select(name,exec) {
         var marker = this.GetObject();
         marker[name] = !marker[name];
         this.execServer(exec + (marker[name] ? '(true)' : '(false)'));
         this.Redraw();
      }
      menu.addchk(marker.fbHasLabel, 'Label', select.bind(this, 'fbHasLabel', 'SetLabelDraw'));
      menu.addchk(marker.fbHasConnector, 'Connector', select.bind(this, 'fbHasConnector', 'SetLineDraw'));
      menu.addchk(marker.fbXDraw, 'Draw X', select.bind(this, 'fbXDraw', 'SetXDraw'));
      menu.addchk(marker.fbYDraw, 'Draw Y', select.bind(this, 'fbYDraw', 'SetYDraw'));
      menu.addchk(marker.fbXbinDraw, 'Draw X bin', select.bind(this, 'fbXbinDraw', 'SetXbinDraw'));
      menu.addchk(marker.fbYbinDraw, 'Draw Y bin', select.bind(this, 'fbYbinDraw', 'SetYbinDraw'));
      menu.addchk(marker.fbContDraw, 'Draw content', select.bind(this, 'fbContDraw', 'SetContDraw'));
      return true;
   }

   GO4.MarkerPainter.prototype.ExtractTooltip = function(pnt) {
      if (!pnt) return null;

      var marker = this.GetObject();

      var hint = { name: marker.fName,
                   title: marker.fTitle,
                   painter: this,
                   menu: true,
                   x: this.grx - (this.frame_x() || 0),
                   y: this.gry - (this.frame_y() || 0),
                   color1: this.markeratt.color };

      var dist = Math.sqrt(Math.pow(pnt.x - hint.x, 2) + Math.pow(pnt.y - hint.y, 2));

      hint.menu_dist = dist;

      if (dist < 2.5 * this.markeratt.GetFullSize()) hint.exact = true;

      if (hint.exact)
         hint.lines = this.fillLabels(marker);

      // res.menu = res.exact; // activate menu only when exactly locate bin
      // res.menu_dist = 3; // distance always fixed

      if (pnt.click_handler && hint.exact)
         hint.click_handler = this.InvokeClickHandler.bind(this);

      return hint;
   }

   GO4.MarkerPainter.prototype.movePntHandler = function() {
      var pos = d3.mouse(this.svg_frame().node()),
          main = this.frame_painter(),
          marker = this.GetObject();

      marker.fX = main.RevertX(pos[0] + this.delta_x);
      marker.fY = main.RevertY(pos[1] + this.delta_y);

      this.drawMarker(true);
   }

   GO4.MarkerPainter.prototype.execServer = function(exec) {
      if (this.snapid && exec) {
         var canp = this.canv_painter();
         if (canp && (typeof canp.SendWebsocket == 'function'))
            canp.SendWebsocket("OBJEXEC:" + this.snapid + ":" + exec);
      }
   }

   GO4.MarkerPainter.prototype.endPntHandler = function() {
      d3.select(window).on("mousemove.markerPnt", null)
                       .on("mouseup.markerPnt", null);

      var marker = this.GetObject();
      if (marker)
         this.execServer("SetXY(" + marker.fX + "," + marker.fY + ")");
      this.drawLabel();
   }

   GO4.MarkerPainter.prototype.InvokeClickHandler = function(hint) {
      if (!hint.exact) return; //

      d3.select(window).on("mousemove.markerPnt", this.movePntHandler.bind(this))
                       .on("mouseup.markerPnt", this.endPntHandler.bind(this), true);

      // coordinate in the frame
      var pos = d3.mouse(this.svg_frame().node());
      this.delta_x = this.grx - pos[0] - this.frame_x();
      this.delta_y = this.gry - pos[1] - this.frame_y();
   }


   GO4.MarkerPainter.prototype.ShowTooltip = function(hint) {
   }

   GO4.drawGo4Marker = function(divid, obj, option) {
      var painter = new GO4.MarkerPainter(obj);
      painter.SetDivId(divid);
      painter.drawMarker();
      painter.drawLabel();
      return painter.DrawingReady();
   }

   // =========================================================================

   GO4.ConditionPainter = function(cond) {
      JSROOT.TObjectPainter.call(this, cond);
      this.cond = cond;
      this.pave = null; // drawing of stat
   }

   GO4.ConditionPainter.prototype = Object.create(JSROOT.TObjectPainter.prototype);

   GO4.ConditionPainter.prototype.Test = function(x,y) {
    // JAM: need to put this here, since condition object will lose internal definition after cloning it again!
    var cond=this.cond;
    if (!cond.fbEnabled) return cond.fbResult;

       if (cond.fxCut)
          return cond.fxCut.IsInside(x,y) ? cond.fbTrue : cond.fbFalse;

       if ((x < cond.fLow1) || (x > cond.fUp1)) return cond.fbFalse;

       if (cond.fiDim==2)
          if ((y < cond.fLow2) || (y > cond.fUp2)) return cond.fbFalse;

       return cond.fbTrue;
    }


   GO4.ConditionPainter.prototype.GetObject = function() {
      return this.cond;
   }

   GO4.ConditionPainter.prototype.isPolyCond = function() {
      return ((this.cond._typename == "TGo4PolyCond") || (this.cond._typename == "TGo4ShapedCond"));
   }

   GO4.ConditionPainter.prototype.isEllipseCond = function() {
      return (this.cond._typename == "TGo4ShapedCond");
   }


   GO4.ConditionPainter.prototype.drawCondition = function() {

      if (this.isPolyCond()) {
         if (this.cond.fxCut != null) {
            // look here if cut is already drawn in divid:
            var cutfound=false;
            var cut=this.cond.fxCut;
            this.ForEachPainter(function(p) {
                if (p.obj_typename != "TCutG") return;
                console.log("Find TCutG painter");
                p.UpdateObject(cut);
                p.Redraw();
                cutfound=true;
             });
            if(cutfound) return;
            // only redraw if previous cut display was not there:
            this.cond.fxCut.fFillStyle = 3006;
            this.cond.fxCut.fFillColor = 2;
            JSROOT.draw(this.divid, this.cond.fxCut, "LF");
         }
         return;
      }

      this.CreateG(true);

      var w = this.frame_width(),
          h = this.frame_height(),
          main = this.main_painter();

      if (!main.grx || !main.gry) main = this.frame_painter();

      if ((this.cond.fFillStyle==1001) && (this.cond.fFillColor==19)) {
         this.cond.fFillStyle = 3006;
         this.cond.fFillColor = 2;
      }

      var fill = this.createAttFill(this.cond);
      var line = this.createAttLine(this.cond);

      this.draw_g.attr("class","cond_container");

      var ndim = this.cond.fiDim;

      this.draw_g.append("svg:rect")
             .attr("x", main.grx(this.cond.fLow1))
             .attr("y", (ndim==1) ? 0 : main.gry(this.cond.fUp2))
             .attr("width", main.grx(this.cond.fUp1) - main.grx(this.cond.fLow1))
             .attr("height", (ndim==1) ? h : main.gry(this.cond.fLow2) - main.gry(this.cond.fUp2))
             .call(line.func)
             .call(fill.func);
   }

   GO4.ConditionPainter.prototype.drawLabel = function() {
      if (!this.cond.fbLabelDraw) return;

      var pave_painter = this.FindPainterFor(this.pave);

      if (!pave_painter) {
         this.pave = JSROOT.Create("TPaveStats");
         this.pave.fName = "stats_" + this.cond.fName;
         JSROOT.extend(this.pave, { fX1NDC: 0.1, fY1NDC: 0.4, fX2NDC: 0.4, fY2NDC: 0.65, fBorderSize: 1, fFillColor: 0, fFillStyle: 1001 });

         var st = JSROOT.gStyle;
         JSROOT.extend(this.pave, { fFillColor: st.fStatColor, fFillStyle: st.fStatStyle, fTextAngle: 0, fTextSize: st.fStatFontSize,
                                    fTextAlign: 12, fTextColor: st.fStatTextColor, fTextFont: st.fStatFont});
      } else {
         this.pave.Clear();
      }

      this.pave.AddText(this.cond.fName);

      this.pave.AddText("Counts = " + this.cond.fiCounts);

// ComputeRange has disappeared from JSROOTcore since 2015 JAM
//      if ((obj_typename.indexOf("TGraph") == 0) || (obj_typename == "TCutG")) {
//          obj['ComputeRange'] = function() {
//             // Compute the x/y range of the points in this graph
//             var res = { xmin: 0, xmax: 0, ymin: 0, ymax: 0 };
//             if (this['fNpoints'] > 0) {
//                res.xmin = res.xmax = this['fX'][0];
//                res.ymin = res.ymax = this['fY'][0];
//                for (var i=1; i<this['fNpoints']; i++) {
//                   if (this['fX'][i] < res.xmin) res.xmin = this['fX'][i];
//                   if (this['fX'][i] > res.xmax) res.xmax = this['fX'][i];
//                   if (this['fY'][i] < res.ymin) res.ymin = this['fY'][i];
//                   if (this['fY'][i] > res.ymax) res.ymax = this['fY'][i];
//                }
//             }
//             return res;
// };
// }


      if (this.cond.fbLimitsDraw)
         if (this.isPolyCond()) {
            //var r = this.cond.fxCut.ComputeRange();
          // ComputeRange has disappeared from JSROOTcore since 2015 JAM2019
          // we implement it here explicitely:
          var res = { xmin: 0, xmax: 0, ymin: 0, ymax: 0 };
          if (this.cond.fxCut['fNpoints'] > 0) {
             res.xmin = res.xmax = this.cond.fxCut['fX'][0];
             res.ymin = res.ymax = this.cond.fxCut['fY'][0];
             for (var i=1; i<this.cond.fxCut['fNpoints']; i++) {
                if (this.cond.fxCut['fX'][i] < res.xmin) res.xmin = this.cond.fxCut['fX'][i];
                if (this.cond.fxCut['fX'][i] > res.xmax) res.xmax = this.cond.fxCut['fX'][i];
                if (this.cond.fxCut['fY'][i] < res.ymin) res.ymin = this.cond.fxCut['fY'][i];
                if (this.cond.fxCut['fY'][i] > res.ymax) res.ymax = this.cond.fxCut['fY'][i];
             }
          }
            var r=res;
          // end workaround for ComputeRange
            this.pave.AddText("Xmin = " + r.xmin);
            this.pave.AddText("Xmax = " + r.xmax);
            this.pave.AddText("Ymin = " + r.ymin);
            this.pave.AddText("Ymax = " + r.ymax);
         } else {
            this.pave.AddText("Xmin = " + this.cond.fLow1);
            this.pave.AddText("Xmax = " + this.cond.fUp1);
            if (this.cond.fiDim==2) {
               this.pave.AddText("Ymin = " + this.cond.fLow2);
               this.pave.AddText("Ymax = " + this.cond.fUp2);
            }
         }

      var painter = this;
      var cond = this.cond;

      if (!('FFormat' in JSROOT))
         JSROOT.FFormat = function(value, fmt) {
            if (fmt == '14.7g') return value.toFixed(1);
            return value.toFixed(4);
         }

      var stat = this.main_painter().CountStat(function(x,y) { return painter.Test(x,y); });

      if (this.cond.fbIntDraw) this.pave.AddText("Integral = " + JSROOT.FFormat(stat.integral, "14.7g"));

      if (this.cond.fbXMeanDraw) this.pave.AddText("Mean x = " + JSROOT.FFormat(stat.meanx, "6.4g"));

      if (this.cond.fbXRMSDraw) this.pave.AddText("RMS x = " + JSROOT.FFormat(stat.rmsx, "6.4g"));

      if (this.cond.fiDim==2) {
         if (this.cond.fbYMeanDraw) this.pave.AddText("Mean y = " + JSROOT.FFormat(stat.meany, "6.4g"));
         if (this.cond.fbYRMSDraw) this.pave.AddText("RMS y = " + JSROOT.FFormat(stat.rmsy, "6.4g"));
      }

      if (this.cond.fbXMaxDraw) this.pave.AddText("X max = " + JSROOT.FFormat(stat.xmax, "6.4g"));

      if (this.cond.fiDim==2)
         if (this.cond.fbYMaxDraw) this.pave.AddText("Y max = " + JSROOT.FFormat(stat.ymax, "6.4g"));
      if (this.cond.fbCMaxDraw) this.pave.AddText("C max = " + JSROOT.FFormat(stat.wmax, "14.7g"));

      if (!pave_painter)
         pave_painter = JSROOT.draw(this.divid, this.pave, "");
      else
         pave_painter.Redraw();
   }

   GO4.ConditionPainter.prototype.RedrawObject = function(obj) {
      if (this.UpdateObject(obj))
         this.Redraw(); // no need to redraw complete pad
   }

   GO4.ConditionPainter.prototype.UpdateObject = function(obj) {
      if (obj._typename != this.cond._typename) return false;

      this.cond = JSROOT.clone(obj);

      return true;
   }

   GO4.ConditionPainter.prototype.Redraw = function() {
      this.drawCondition();
      this.drawLabel();
   }

   GO4.drawGo4Cond = function(divid, cond, option) {

      if (GO4.web_canvas || (option=='same')) {
         var condpainter = new GO4.ConditionPainter(cond);
         condpainter.SetDivId(divid);
         condpainter.drawCondition();
         condpainter.drawLabel();
         return condpainter.DrawingReady();
      }

      // from here normal code for plain THttpServer

      if ((cond.fxHistoName=="") || (option=='editor')) {
         // $('#'+divid).append("<br/>Histogram name not specified");
         var h = $("#"+divid).height(), w = $("#"+divid).width();
         if ((h<10) && (w>10)) $("#"+divid).height(w*0.4);
         var editor = new GO4.ConditionEditor(cond);
         return editor.drawEditor(divid);
      }

      // $('#'+divid).append("<br/>Histogram name is " + cond.fxHistoName);

      if (!JSROOT.hpainter) {
         $('#'+divid).append("<br/>Error - did not found hierarchy painter");
         return;
      }

      var histofullpath = null;

      JSROOT.hpainter.ForEach(function(h) {
         if ((h['_name'] == cond.fxHistoName) && (h['_kind'].indexOf("ROOT.TH")==0)) {
            histofullpath = JSROOT.hpainter.itemFullName(h);
            return true;
         }
      });

      if (histofullpath == null) {
         $('#'+divid).append("<br/>Error - did not found histogram " + cond.fxHistoName);

         histofullpath = "../../Histograms/" + cond.fxHistoName;

         JSROOT.hpainter.Find({ name: histofullpath, force: true})['_kind'] = "ROOT.TH1I";

         console.log("Try histogram" + histofullpath);
      }

      $('#'+divid).append("<br/>Drawing histogram " + histofullpath);

      $('#'+divid).empty();

      var condpainter = new GO4.ConditionPainter(cond);

      JSROOT.hpainter.display(histofullpath, "divid:" + divid, function(res) {
         if (res==null) return console.log("fail to get histogram " + histofullpath);
         condpainter.SetDivId(divid);
         condpainter.drawCondition();
         condpainter.drawLabel();
         condpainter.DrawingReady();
      });

      return condpainter;
   }


   // =======================================================================

   if (GO4.web_canvas) {
      JSROOT.addDrawFunc("TGo4Marker", GO4.drawGo4Marker, "");
      JSROOT.addDrawFunc("TGo4WinCond", GO4.drawGo4Cond, "");
      JSROOT.addDrawFunc("TGo4PolyCond", GO4.drawGo4Cond, "");
      JSROOT.addDrawFunc("TGo4ShapedCond", GO4.drawGo4Cond, "");
   }

   return GO4;

}));
