#include "CoilyState.h"
#include "CoilyComponent.h"
#include <cstdlib>
#include <cmath>
#include <climits>

namespace dae
{
    // Pyramid bounds check: row in [0,6], col in [0, row]
    static bool InBounds(int row, int col)
    {
        return row >= 0 && row <= 6 && col >= 0 && col <= row;
    }

    // 0=up-right(row-1,col), 1=up-left(row-1,col-1), 2=down-right(row+1,col+1), 3=down-left(row+1,col)
    static const int dRow[4] = { -1, -1, 1, 1 };
    static const int dCol[4] = { 0, -1, 1, 0 };

    std::unique_ptr<CoilyBaseState> CoilyEggState::Update(float deltaTime, CoilyComponent& coily)
    {
        m_groundTimer += deltaTime;
        if (m_groundTimer < m_hopInterval) return nullptr;
        m_groundTimer = 0.f;

        int row = coily.GetGridRow();
        int col = coily.GetGridCol();

        // Egg only moves down: direction 2 (down-right) or 3 (down-left)
        int dir = (rand() % 2) + 2;
        int newRow = row + dRow[dir];
        int newCol = col + dCol[dir];

        if (!InBounds(newRow, newCol))
        {
            // Try the other down direction
            dir = (dir == 2) ? 3 : 2;
            newRow = row + dRow[dir];
            newCol = col + dCol[dir];
        }

        if (InBounds(newRow, newCol))
            coily.BeginHop(newRow, newCol, m_hopInterval);

        return nullptr;
    }

    std::unique_ptr<CoilyBaseState> CoilyEggState::OnLanded(CoilyComponent& coily)
    {
        if (coily.GetGridRow() == 6)
            return std::make_unique<CoilySnakeState>(m_hopInterval);
        return nullptr;
    }

    int CoilySnakeState::GetCol(bool inAir) const
    {
        int base = 2 + m_direction * 2;
        return inAir ? base + 1 : base;
    }

    std::unique_ptr<CoilyBaseState> CoilySnakeState::Update(float deltaTime, CoilyComponent& coily)
    {
        m_groundTimer += deltaTime;
        if (m_groundTimer < m_hopInterval) return nullptr;
        m_groundTimer = 0.f;

        int row = coily.GetGridRow();
        int col = coily.GetGridCol();
        int targetRow = coily.GetTargetRow();
        int targetCol = coily.GetTargetCol();

        int bestDir = -1;
        int bestDist = INT_MAX;
        for (int dir = 0; dir < 4; ++dir)
        {
            int nr = row + dRow[dir];
            int nc = col + dCol[dir];
            if (!InBounds(nr, nc)) continue;
            int dist = abs(nr - targetRow) + abs(nc - targetCol);
            if (dist < bestDist)
            {
                bestDist = dist;
                bestDir = dir;
            }
        }

        if (bestDir == -1) return nullptr;

        m_direction = bestDir;
        coily.BeginHop(row + dRow[bestDir], col + dCol[bestDir], m_hopInterval);

        return nullptr;
    }

    std::unique_ptr<CoilyBaseState> CoilySnakeState::OnLanded(CoilyComponent&)
    {
        return nullptr;
    }
}