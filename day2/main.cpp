#include <cstdio>
#include <fstream>
#include <string>

struct Strat {
  char m_opponent;
  char m_you;

  int m_opponent_score = 0;
  int m_your_score = 0;

  bool winning_move(int o, int y) {
    return (((o % 3) & y) == y) && (((o % 3) == 0) == (y == 0));
  }

  void calculate_score_v1() {
    m_opponent_score = m_opponent & 3;
    m_your_score = (m_you& 3) + 1;

    bool is_same = m_opponent_score == m_your_score;
    bool you_winner = winning_move(m_opponent & 3, m_you& 3);

    m_opponent_score += is_same ? 3 : (!you_winner ? 6 : 0);
    m_your_score += is_same ? 3 : (you_winner ? 6 : 0);
  }

  void calculate_score_v2() {
    switch (m_you) {
    case 'X':
      m_opponent_score = (m_opponent & 3) + 6;
      for (int i = 0; i < 3; ++i) {
        if ((i + 1) != (m_opponent & 3) && winning_move((i + 1), (m_opponent & 3) - 1)) {
          m_your_score = i + 1;
          break;
        }
      }
      break;
    case 'Y':
      m_opponent_score = (m_opponent & 3) + 3;
      m_your_score = (m_opponent & 3) + 3;
      break;
    case 'Z':
      m_opponent_score = (m_opponent & 3);
      for (int i = 0; i < 3; ++i) {
        if ((i + 1) != (m_opponent & 3) && winning_move((m_opponent & 3), i)) {
          m_your_score = i + 7;
          break;
        }
      }
      break;
    }
  }
};

std::istream &operator>>(std::istream &is, Strat &s) {
  s.m_opponent = is.get();
  is.get();
  s.m_you= is.get();
  is.get();
  return is;
}

int main(void) {
  {
    std::ifstream fs("input.txt", fs.in);

    int myTotal = 0;
    for (Strat s; fs >> s; ) {
      s.calculate_score_v1();
      myTotal += s.m_your_score;
    }

    std::printf("My total v1: %d\n", myTotal);
  }
  {
    std::ifstream fs("input.txt", fs.in);

    int myTotal = 0;
    for (Strat s; fs >> s; ) {
      s.calculate_score_v2();
      myTotal += s.m_your_score;
    }

    std::printf("My total v2: %d\n", myTotal);
  }
  return 0;
}
