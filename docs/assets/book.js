(function(){
'use strict';
var root=document.getElementById('leaves');if(!root)return;
var leaves=[].slice.call(root.children),orig=leaves.map(function(l){return l.cloneNode(true)}),N=leaves.length,KEY='ofs-book-marks',cur=0,busy=false,drag=null;
var lang=document.body.dataset.lang||'pt';
var marks=[];try{marks=JSON.parse(localStorage.getItem(KEY)||'[]')}catch(e){}
function saveMarks(){try{localStorage.setItem(KEY,JSON.stringify(marks))}catch(e){}}
function key(i){var l=leaves[i];return lang+':'+(l?l.dataset.o+'-'+l.dataset.part:i)}
function idxOfKey(k){for(var i=0;i<leaves.length;i++)if(key(i)===k)return i;return -1}
var rmq=window.matchMedia('(prefers-reduced-motion:reduce)');
function esc(s){return String(s).replace(/[&<>"]/g,function(c){return {'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;'}[c]})}
function div(c){var d=document.createElement('div');d.className=c;return d}
function btn(c,t,l){var b=document.createElement('button');b.className=c;b.textContent=t;b.setAttribute('aria-label',l);return b}

// ---- Estrutura 3D do livro (Referência Codrops 3D)
var main=document.getElementById('book');var VER=((document.querySelector('[data-relver]')||{}).textContent||'').replace(/^v/,'');
var stage=div('stage');
var book=div('book closed');
book.innerHTML=[
  '<div class="ground"></div>',
  '<ul class="hardcover_front">',
  '  <li>',
  '    <span class="ribbon">v'+VER+'</span>',
  '    <div class="coverDesign"></div>',
  '  </li>',
  '  <li>',
  '    <div class="insideCover"></div>',
  '  </li>',
  '</ul>',
  '<ul class="page">',
  '  <li class="page-layer p1"></li>',
  '  <li class="page-layer p2"></li>',
  '  <li class="page-layer p3"></li>',
  '  <li class="page-layer p4"></li>',
  '  <li class="page-layer p5"></li>',
  '</ul>',
  '<ul class="hardcover_back">',
  '  <li></li>',
  '  <li></li>',
  '</ul>',
  '<ul class="book_spine">',
  '  <li></li>',
  '  <li></li>',
  '</ul>'
].join('');

var frontCover=book.querySelector('.coverDesign');
var insideCover=book.querySelector('.insideCover');
var pageLayers=book.querySelectorAll('.page > li');
var p2=pageLayers[1]; // Active reading layer (recto)

// Render cover content
if(leaves[0]){
  frontCover.innerHTML=leaves[0].innerHTML;
}

// Active and verso pages
var L=mkPage(),R=mkPage();
insideCover.appendChild(L.el);
p2.appendChild(R.el);

// Grab areas for turning
var gl=div('grab l'),gr=div('grab r');
insideCover.appendChild(gl);
p2.appendChild(gr);

// Chapter tabs (stabs)
var stabs=div('stabs');
stage.appendChild(stabs);

stage.appendChild(book);
var prevB=btn('nav-btn prev','‹','‹'),nextB=btn('nav-btn next','›','›');
stage.appendChild(prevB);stage.appendChild(nextB);
main.appendChild(stage);

function mkPage(){
  var el=div('pg');
  el.innerHTML='<div class="body"></div><div class="num"></div><button class="rib" aria-label="Marcador" aria-pressed="false"></button>';
  return {el:el,body:el.children[0],num:el.children[1],rib:el.children[2]};
}

function clean(p){
  p.body.querySelectorAll('[id]').forEach(function(e){e.removeAttribute('id')});
}

function fill(p,i,mode){
  var el=p.el;
  el.classList.remove('verso','endpaper','cvr');
  if(mode==='e'||i<0){
    el.classList.add('endpaper');
    p.body.innerHTML='<div class="leaf-in" style="text-align:center;padding-top:40%;color:var(--muted);"><p style="font-size:1.8rem;letter-spacing:.1em;">✦ OFS ✦</p><p style="font-size:0.95rem;">Obsidian Fault Script Documentation</p></div>';
    p.num.textContent='';
    return;
  }
  p.body.innerHTML=leaves[i].outerHTML;
  p.num.textContent=String(i+1);
  if(mode==='v')el.classList.add('verso');
  if(i===0&&mode!=='v')el.classList.add('cvr');
  var m=marks.indexOf(key(i))>=0;
  p.rib.setAttribute('aria-pressed',m?'true':'false');
  p.rib.title=m?'Remover marcador':'Marcar página';
  clean(p);
}

function leftFor(c){
  if(c<=1)fill(L,-1,'e');
  else fill(L,c-1,'v');
}

function setBookState(c){
  var isClosed = (c === 0);
  book.classList.toggle('closed', isClosed);
  book.classList.toggle('open', !isClosed);
}

// Chapters & tabs
var chapters=[];
function calcChapters(){chapters=[];leaves.forEach(function(l,i){
  if(l.classList.contains('chapter-page'))chapters.push({i:i,t:l.dataset.t});
})}
calcChapters();

function render(){
  fill(R,cur,'n');
  leftFor(cur);
  setBookState(cur);
  prevB.disabled=(cur<=0);
  nextB.disabled=(cur>=N-1);
  where.textContent=(cur+1)+' / '+N;
  history.replaceState(null,'','#p'+(cur+1));
  try{localStorage.setItem('ofs-book-last:'+lang,cur)}catch(e){}
  buildStabs();
}

function buildStabs(){
  var h='',act=-1;
  chapters.forEach(function(c,k){if(c.i<=cur)act=k});
  chapters.forEach(function(c,k){
    h+='<button class="stab'+(k===act?' on':'')+'" style="--h:'+(255+k*13)+'" data-i="'+c.i+'" title="'+esc(c.t)+'" aria-label="'+esc(c.t)+'"><span class="n">'+(k+1)+'</span><span class="t"> '+esc(c.t)+'</span></button>';
  });
  marks.forEach(function(k){
    var p=k.split(':');if(p[0]!==lang)return;
    var i=idxOfKey(k);if(i<0)return;
    var t=leaves[i].dataset.t||'';
    h+='<button class="stab mk" data-i="'+i+'" title="'+esc(t)+'" aria-label="'+esc(t)+'"><span class="n">★</span><span class="t"> '+(i+1)+' '+esc(t)+'</span></button>';
  });
  stabs.innerHTML=h;
}
stabs.addEventListener('click',function(e){
  var b=e.target.closest('.stab');if(b)jump(+b.dataset.i);
});

// ---- Virada de página com física 3D e sombra
function startFlip(dir){
  var t=cur+dir;
  if(t<0||t>=N)return null;
  var idx=dir>0?cur:t;
  var f=div('flip'),fr=div('face front'),bk=div('face back'),sh=div('shade'),sh2=div('shade');
  var a=mkPage(),b=mkPage();
  fill(a,idx,'n');fr.appendChild(a.el);fr.appendChild(sh);
  if(idx===0)fill(b,-1,'e');else fill(b,idx,'v');
  bk.appendChild(b.el);bk.appendChild(sh2);
  f.appendChild(fr);f.appendChild(bk);

  if(dir>0){
    fill(R,t,'n');
    if(cur===0)leftFor(1);
  } else {
    leftFor(t);
  }
  setBookState(t);
  book.querySelector('.page').appendChild(f);
  return {f:f,sh:sh,sh2:sh2,dir:dir,t:t};
}

function setP(fl,p){
  // No livro aberto: ângulo vai de -35deg até -135deg
  var fromAng = fl.dir > 0 ? -35 : -135;
  var toAng = fl.dir > 0 ? -135 : -35;
  var ang = fromAng + (toAng - fromAng) * p;
  fl.f.style.transform='rotateY('+ang+'deg)';
  var q=Math.max(0,Math.min(1,p));
  var s=Math.sin(Math.PI*q);
  fl.sh.style.opacity=s*0.75;
  fl.sh2.style.opacity=s*0.75;
}

function ease(k){
  if(k<.76){var u=k/.76;return 1.035*Math.pow(u,2.1)}
  var v=(k-.76)/.24;
  return 1.035-.035*(1-Math.cos(Math.PI*v))/2;
}

function settle(fl,from,to,ms,done){
  var t0=null;
  function end(){
    fl.f.remove();
    if(to>=0.5)cur=fl.t;
    render();
    done&&done();
  }
  if(rmq.matches||ms<=0){end();return}
  function step(ts){
    if(t0===null)t0=ts;
    var k=Math.min(1,(ts-t0)/ms),e;
    if(to>=0.5){e=from+(1-from)*ease(k)}
    else{e=from*(1-(1-Math.pow(1-k,3)))}
    setP(fl,e);
    if(k<1)requestAnimationFrame(step);
    else end();
  }
  requestAnimationFrame(step);
}

function turn(dir,ms,done){
  if(busy||drag)return;
  var fl=startFlip(dir);
  if(!fl){done&&done();return}
  busy=true;
  setP(fl,0);
  settle(fl,0,1,ms==null?850:ms,function(){busy=false;done&&done()});
}

function go(n){
  n=Math.max(0,Math.min(N-1,n));
  if(n===cur)return;
  turn(n>cur?1:-1);
}

function jump(n){
  n=Math.max(0,Math.min(N-1,n));
  if(n===cur||busy)return;
  if(rmq.matches){cur=n;render();return}
  var dir=n>cur?1:-1,steps=Math.min(6,Math.abs(n-cur));
  cur=n-dir*steps;
  render();
  (function next(k){
    if(k>=steps)return;
    turn(dir,140,function(){next(k+1)});
  })(0);
}

// Arrasto manual
function dragStart(e,dir){
  if(busy||drag)return;
  var fl=startFlip(dir);if(!fl)return;
  e.preventDefault();
  var w=p2.getBoundingClientRect().width;
  drag={fl:fl,x:e.clientX,w:w,p:0,tgt:0,id:e.pointerId,raf:0};
  try{e.currentTarget.setPointerCapture(e.pointerId)}catch(_){}
  setP(fl,0);
  (function loop(){
    if(!drag)return;
    drag.p+=(drag.tgt-drag.p)*0.22;
    setP(drag.fl,drag.p);
    drag.raf=requestAnimationFrame(loop);
  })();
}

function dragMove(e){
  if(!drag||e.pointerId!==drag.id)return;
  var dx=e.clientX-drag.x,d=drag.fl.dir,r=Math.max(0,(d>0?-dx:dx)/(drag.w*0.92));
  drag.tgt=Math.min(1,Math.pow(r,1.1)*0.95);
}

function dragEnd(e){
  if(!drag||e.pointerId!==drag.id)return;
  var fl=drag.fl,p=drag.p,done=drag.tgt>0.42;
  cancelAnimationFrame(drag.raf);
  drag=null;busy=true;
  settle(fl,p,done?1:0,done?(800*(1-p)+200):(350*p+100),function(){busy=false});
}

[[gr,1],[gl,-1]].forEach(function(a){
  var el=a[0],dir=a[1];
  el.addEventListener('pointerdown',function(e){dragStart(e,dir)});
  el.addEventListener('pointermove',dragMove);
  el.addEventListener('pointerup',dragEnd);
  el.addEventListener('pointercancel',dragEnd);
});

// Sumário lateral e barra inferior
var side=document.createElement('aside');side.className='side';document.body.appendChild(side);
function buildSide(){
  var h='<button class="close" aria-label="Fechar">×</button><h2>'+document.body.dataset.toc+'</h2>',last=null;
  leaves.forEach(function(l,i){
    var c=l.dataset.ch,t=l.dataset.t;if(!c&&i>1)return;
    if(c&&c!==last){h+='<div class="ch">'+esc(c)+'</div>';last=c}
    if(l.classList.contains('chapter-page'))return;
    h+='<a href="#p'+(i+1)+'" data-i="'+i+'"><span>'+esc(t)+'</span><span class="pn">'+(i+1)+'</span></a>';
  });
  side.innerHTML=h;
}
side.addEventListener('click',function(e){
  var a=e.target.closest('a[data-i]');
  if(a){e.preventDefault();side.classList.remove('open');jump(+a.dataset.i);return}
  if(e.target.closest('.close'))side.classList.remove('open');
});

var tabs=div('tabs'),bIdx=btn2('☰ '+document.body.dataset.toc),bPrev=btn2('‹'),where=document.createElement('span'),bNext=btn2('›');
where.className='where';
function btn2(t){var b=document.createElement('button');b.textContent=t;return b}
tabs.appendChild(bIdx);tabs.appendChild(bPrev);tabs.appendChild(where);tabs.appendChild(bNext);
document.body.appendChild(tabs);

bIdx.onclick=function(){side.classList.toggle('open')};
bPrev.onclick=prevB.onclick=function(){go(cur-1)};
bNext.onclick=nextB.onclick=function(){go(cur+1)};

// Interações gerais (marcador, links, abrir livro)
stage.addEventListener('click',function(e){
  var r=e.target.closest('.rib');
  if(r){
    var k=key(cur),j=marks.indexOf(k);
    if(j>=0)marks.splice(j,1);else marks.push(k);
    saveMarks();
    fill(R,cur,'n');
    buildStabs();
    return;
  }
  var bx=e.target.closest('.box');
  if(bx){toggleBox();return}
  var ob=e.target.closest('.open-book');
  if(ob){
    e.preventDefault();
    go(1);
    return;
  }
  var a=e.target.closest('a[href^="#"]');
  if(a){
    e.preventDefault();
    route(a.getAttribute('href').slice(1));
  }
});

document.addEventListener('keydown',function(e){
  if((e.key==='Enter'||e.key===' ')&&e.target.classList&&e.target.classList.contains('box')){
    e.preventDefault();toggleBox();
  }
});

var boxOpen=false;
function applyBox(){
  document.querySelectorAll('.relcard').forEach(function(r){
    r.classList.toggle('open',boxOpen);
    var b=r.querySelector('.box');
    if(b)b.setAttribute('aria-expanded',boxOpen?'true':'false');
  });
}
function toggleBox(){boxOpen=!boxOpen;applyBox()}

function route(id){
  var m=/^p(\d+)$/.exec(id);
  if(m){go(+m[1]-1);return}
  var el=document.getElementById(id)||root.querySelector('[id="'+id+'"]');
  if(el){
    var lf=el;
    while(lf&&lf.parentNode!==root)lf=lf.parentNode;
    if(lf)jump(leaves.indexOf(lf));
  }
}

var pop=document.getElementById('dlpop'),dlb=document.querySelector('.dlbtn');
if(dlb&&pop){
  dlb.onclick=function(){
    var h=pop.hidden;pop.hidden=!h;dlb.setAttribute('aria-expanded',h?'true':'false');
  };
  pop.addEventListener('click',function(e){if(e.target.closest('.box'))toggleBox()});
}
if(rmq.matches){boxOpen=true;applyBox()}

// Versão do release
try{
  fetch('https://api.github.com/repos/Samwns/Obsidian-Fault-Script/releases/latest',{headers:{Accept:'application/vnd.github+json'}})
  .then(function(r){return r.ok?r.json():null})
  .then(function(j){
    if(!j||!j.tag_name)return;
    document.querySelectorAll('[data-relver]').forEach(function(n){n.textContent=j.tag_name});
    document.querySelectorAll('.ribbon').forEach(function(n){n.textContent=j.tag_name});
    document.querySelectorAll('[data-relsrc]').forEach(function(n){n.remove()});
    document.querySelectorAll('a.dl').forEach(function(a){
      var pre=a.dataset.asset,ext=a.dataset.ext;
      var m=(j.assets||[]).filter(function(x){return (pre?x.name.indexOf(pre)===0:true)&&x.name.slice(-ext.length)===ext})[0];
      if(m)a.href=m.browser_download_url;
    });
    fill(R,cur,'n');
    applyBox();
  }).catch(function(){});
}catch(e){}

// Teclado e touch swipe
document.addEventListener('keydown',function(e){
  if(/INPUT|TEXTAREA|SELECT/.test((e.target.tagName||'')))return;
  if(e.key==='ArrowRight'||e.key==='PageDown')go(cur+1);
  else if(e.key==='ArrowLeft'||e.key==='PageUp')go(cur-1);
  else if(e.key==='Home')jump(0);
  else if(e.key==='End')jump(N-1);
  else if(e.key==='Escape'){side.classList.remove('open');if(pop)pop.hidden=true}
});

var sx=0,sy=0;
book.addEventListener('touchstart',function(e){sx=e.touches[0].clientX;sy=e.touches[0].clientY},{passive:true});
book.addEventListener('touchend',function(e){
  if(drag||busy)return;
  var dx=e.changedTouches[0].clientX-sx,dy=e.changedTouches[0].clientY-sy;
  if(Math.abs(dx)>60&&Math.abs(dx)>Math.abs(dy)*1.6)go(cur+(dx<0?1:-1));
},{passive:true});

window.addEventListener('hashchange',function(){route(location.hash.slice(1))});
function tocNums(){
  root.querySelectorAll('.toc-doc a').forEach(function(a){
    var t=document.getElementById(a.getAttribute('href').slice(1));
    while(t&&t.parentNode!==root)t=t.parentNode;
    var o=a.parentNode.querySelector('.pn');if(o)o.remove();
    if(t){
      var s=document.createElement('span');s.className='pn';s.textContent=leaves.indexOf(t)+1;
      a.parentNode.appendChild(s);
    }
  });
}

// ---- Paginação pelo tamanho real da página
var meas=null,ready=false,lastSig='',rt=null;
function paginate(){
  if(!meas){
    meas=mkPage();meas.el.style.cssText='visibility:hidden;pointer-events:none;z-index:-1';
    p2.appendChild(meas.el);
  }
  var B=meas.body,out=[];
  function fits(){return B.scrollHeight<=B.clientHeight+1}
  function mk(src,part){
    var a=src.cloneNode(false);a.removeAttribute('id');
    a.dataset.o=src.dataset.o;a.dataset.part=part;
    var inn=document.createElement('div');inn.className='leaf-in';a.appendChild(inn);
    return a;
  }
  orig.forEach(function(src,oi){
    var keep=oi===0||src.classList.contains('chapter-page')||!src.querySelector('.leaf-in');
    if(keep){var c=src.cloneNode(true);c.dataset.o=oi;c.dataset.part=0;out.push(c);return}
    src.dataset.o=oi;
    var kids=[].slice.call(src.querySelector('.leaf-in').children).map(function(k){return k.cloneNode(true)});
    var part=0,cp=mk(src,part),inn=cp.firstChild;
    if(src.id)cp.id=src.id;
    B.innerHTML='';B.appendChild(cp);
    function next(){
      if(!inn.children.length)return;
      out.push(cp);part++;cp=mk(src,part);inn=cp.firstChild;
      B.innerHTML='';B.appendChild(cp);
    }
    function add(k){
      inn.appendChild(k);
      if(fits())return true;
      inn.removeChild(k);return false;
    }
    function splitK(k){
      var tag=k.tagName,items=null,mkc=null;
      if(tag==='PRE'){
        var code=k.querySelector('code')||k;
        items=code.textContent.split('\n');
        mkc=function(ls){var p=k.cloneNode(false),c=document.createElement('code');c.textContent=ls.join('\n');p.appendChild(c);return p};
      }else if(tag==='UL'||tag==='OL'){
        items=[].slice.call(k.children);
        mkc=function(ls){var u=k.cloneNode(false);ls.forEach(function(x){u.appendChild(x.cloneNode(true))});return u};
      }else if(k.classList.contains('tw')&&k.querySelector('tbody')){
        var tb=k.querySelector('tbody'),tbl=k.querySelector('table');
        items=[].slice.call(tb.children);
        mkc=function(ls){var w=k.cloneNode(false),t=tbl.cloneNode(false);
          var th=tbl.querySelector('thead');if(th)t.appendChild(th.cloneNode(true));
          var b=tb.cloneNode(false);ls.forEach(function(x){b.appendChild(x.cloneNode(true))});t.appendChild(b);w.appendChild(t);return w};
      }
      if(!items||items.length<2){inn.appendChild(k);return}
      var buf=[],node=null;
      for(var q=0;q<items.length;q++){
        buf.push(items[q]);
        var nn=mkc(buf);
        if(node)inn.removeChild(node);
        inn.appendChild(nn);node=nn;
        if(!fits()&&buf.length>1){
          inn.removeChild(nn);buf.pop();
          node=null;
          if(buf.length)inn.appendChild(mkc(buf));
          next();buf=[items[q]];
          node=mkc(buf);inn.appendChild(node);
        }
      }
    }
    kids.forEach(function(k){
      if(add(k))return;
      var splittable=/^(PRE|UL|OL)$/.test(k.tagName)||k.classList.contains('tw');
      if(splittable){splitK(k);return}
      if(inn.children.length){next();if(add(k))return}
      inn.appendChild(k);
    });
    out.push(cp);
  });
  B.innerHTML='';
  var keepO=leaves[cur]?leaves[cur].dataset.o:'0',keepP=leaves[cur]?leaves[cur].dataset.part:'0';
  root.innerHTML='';out.forEach(function(l){root.appendChild(l)});
  leaves=out;N=leaves.length;
  var ni=0;
  for(var i=0;i<N;i++){if(leaves[i].dataset.o===keepO){if(!ni||leaves[i].dataset.part===keepP)ni=i;if(leaves[i].dataset.part===keepP)break}}
  cur=ni;
}
function repaginate(){
  paginate();calcChapters();tocNums();buildSide();
  cur=Math.max(0,Math.min(N-1,cur));
  lastSig=p2.clientWidth+'x'+p2.clientHeight;
  render();
}
window.addEventListener('resize',function(){
  clearTimeout(rt);rt=setTimeout(function(){
    if(p2.clientWidth+'x'+p2.clientHeight!==lastSig&&!busy&&!drag)repaginate();
  },200);
});
if(document.fonts&&document.fonts.ready)document.fonts.ready.then(function(){if(ready)repaginate()});

var start=0,m=/^#p(\d+)$/.exec(location.hash);
if(m)start=+m[1]-1;
else try{start=+localStorage.getItem('ofs-book-last:'+lang)||0}catch(e){}
cur=0;paginate();calcChapters();tocNums();buildSide();ready=true;
cur=Math.max(0,Math.min(N-1,start));
lastSig=p2.clientWidth+'x'+p2.clientHeight;
render();
if(location.hash&&!m)route(location.hash.slice(1));
})();
