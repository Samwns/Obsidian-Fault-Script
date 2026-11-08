// Wiki OFS: busca, "[esconder]" do indice e pagina aleatoria. Tudo opcional.
(function () {
  var q = document.getElementById('wk-q'), res = document.getElementById('wk-results');
  var idx = null, loading = false;
  function load(cb) {
    if (idx) return cb();
    if (loading) return;
    loading = true;
    fetch('search.json').then(function (r) { return r.json(); }).then(function (j) { idx = j; cb(); }).catch(function () { idx = []; });
  }
  function run() {
    var t = q.value.trim().toLowerCase();
    res.innerHTML = '';
    if (!t) return;
    var n = 0;
    idx.forEach(function (p) {
      if (n >= 12) return;
      var hit = p.t.toLowerCase().indexOf(t) >= 0 ? p.t : '';
      if (!hit) p.s.forEach(function (s) { if (!hit && s.toLowerCase().indexOf(t) >= 0) hit = p.t + ' › ' + s; });
      if (!hit) return;
      var li = document.createElement('li'), a = document.createElement('a');
      a.href = p.u; a.textContent = hit; li.appendChild(a); res.appendChild(li); n++;
    });
  }
  if (q && res) {
    q.addEventListener('input', function () { load(run); });
    var m = /[?&]q=([^&]*)/.exec(location.search);
    if (m) { q.value = decodeURIComponent(m[1].replace(/\+/g, ' ')); load(run); }
  }
  var b = document.getElementById('toc-btn'), toc = document.getElementById('toc');
  if (b && toc) b.addEventListener('click', function (e) {
    e.preventDefault();
    var h = toc.classList.toggle('closed');
    b.textContent = b.getAttribute(h ? 'data-show' : 'data-hide');
  });
  var r = document.getElementById('wk-random');
  if (r && window.WK_PAGES) r.addEventListener('click', function () {
    r.href = WK_PAGES[Math.floor(Math.random() * WK_PAGES.length)];
  });
})();
