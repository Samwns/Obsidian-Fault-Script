document.querySelectorAll('.card').forEach((card, index) => {
  card.style.animation = `enter .45s ease ${index * 80}ms both`;
});
