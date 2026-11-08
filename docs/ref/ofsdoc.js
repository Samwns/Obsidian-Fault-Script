// ofsdoc: busca, tema claro/escuro e gaveta da lateral. Tudo opcional (a pagina
// funciona sem JS; a gaveta mobile cai para o link #side).
(function () {
  var root = document.body.getAttribute('data-root') || '';
  var side = document.getElementById('side');

  // tema
  var tb = document.getElementById('theme');
  if (tb) tb.addEventListener('click', function () {
    var el = document.documentElement, cur = el.getAttribute('data-theme');
    if (!cur) cur = window.matchMedia('(prefers-color-scheme: light)').matches ? 'light' : 'dark';
    var next = cur === 'dark' ? 'light' : 'dark';
    el.setAttribute('data-theme', next);
    try { localStorage.setItem('ofsdoc-theme', next); } catch (e) {}
  });

  // gaveta (mobile)
  var menu = document.getElementById('menu');
  function closeNav() { document.body.classList.remove('nav-open'); if (location.hash === '#side') history.replaceState(null, '', location.pathname + location.search); }
  if (menu && side) {
    menu.addEventListener('click', function (e) { e.preventDefault(); document.body.classList.toggle('nav-open'); });
    side.addEventListener('click', function (e) { if (e.target.closest && e.target.closest('a')) closeNav(); });
    document.addEventListener('click', function (e) {
      if (document.body.classList.contains('nav-open') && !side.contains(e.target) && !menu.contains(e.target)) closeNav();
    });
    document.addEventListener('keydown', function (e) { if (e.key === 'Escape') closeNav(); });
  }
  var cur = side && side.querySelector('a.on');
  if (cur && side) { try { side.scrollTop = Math.max(0, cur.offsetTop - side.clientHeight / 3); } catch (e) {} }

  // busca
  var q = document.getElementById('q'), res = document.getElementById('res');
  if (!q || !res) return;
  var idx = null, loading = false, sel = -1;
  function load(cb) {
    if (idx) return cb();
    if (loading) return;
    loading = true;
    fetch(root + 'search.json').then(function (r) { return r.json(); }).then(function (j) { idx = j; cb(); }).catch(function () { idx = []; cb(); });
  }
  function score(p, t) {
    var n = p.n.toLowerCase(), s = 0;
    if (n === t) s = 100;
    else if (n.indexOf(t) === 0) s = 80;
    else if (n.lastIndexOf('.' + t) >= 0 && n.lastIndexOf('.' + t) === n.length - t.length - 1) s = 75;
    else if (n.indexOf(t) >= 0) s = 60;
    else if ((p.m || '').toLowerCase().indexOf(t) >= 0) s = 35;
    else if ((p.s || '').toLowerCase().indexOf(t) >= 0) s = 25;
    else if ((p.d || '').toLowerCase().indexOf(t) >= 0) s = 15;
    if (s && p.k === 'module') s += 6;
    if (s && p.k === 'doc') s += 4;
    return s;
  }
  function esc(x) { return String(x).replace(/[&<>"]/g, function (c) { return { '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]; }); }
  function run() {
    var t = q.value.trim().toLowerCase();
    res.innerHTML = ''; sel = -1;
    if (!t) { res.hidden = true; return; }
    var hits = [];
    idx.forEach(function (p) { var s = score(p, t); if (s) hits.push([s, p]); });
    hits.sort(function (a, b) { return b[0] - a[0] || a[1].n.length - b[1].n.length; });
    res.hidden = false;
    if (!hits.length) { res.innerHTML = '<li class="none">…</li>'; return; }
    res.innerHTML = hits.slice(0, 30).map(function (h) {
      var p = h[1], sub = p.k === 'doc' ? (p.s || '') : (p.s || p.d || '');
      return '<li><a href="' + root + esc(p.u) + '"><b>' + esc(p.n) + '</b><em>' + esc(p.k) + '</em><span>' + esc(sub) + '</span></a></li>';
    }).join('');
  }
  function items() { return res.querySelectorAll('a'); }
  function mark(i) {
    var a = items();
    if (!a.length) return;
    sel = (i + a.length) % a.length;
    for (var k = 0; k < a.length; k++) a[k].classList.toggle('sel', k === sel);
    a[sel].scrollIntoView({ block: 'nearest' });
  }
  q.addEventListener('input', function () { load(run); });
  q.addEventListener('focus', function () { load(function () { if (q.value) run(); }); });
  q.addEventListener('keydown', function (e) {
    if (e.key === 'ArrowDown') { e.preventDefault(); mark(sel + 1); }
    else if (e.key === 'ArrowUp') { e.preventDefault(); mark(sel - 1); }
    else if (e.key === 'Enter') { var a = items(); if (a.length) { e.preventDefault(); (a[sel >= 0 ? sel : 0]).click(); } }
    else if (e.key === 'Escape') { q.value = ''; res.hidden = true; q.blur(); }
  });
  var pm = /[?&]q=([^&]*)/.exec(location.search);
  if (pm) { q.value = decodeURIComponent(pm[1].replace(/\+/g, ' ')); load(run); }
  document.addEventListener('keydown', function (e) {
    if (e.key === '/' && !/INPUT|TEXTAREA/.test((document.activeElement || {}).tagName || '')) {
      e.preventDefault();
      document.body.classList.add('nav-open');
      q.focus();
    }
  });
})();
