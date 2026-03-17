# Contributing Guide

## 목표

- 팀원 모두가 같은 방식으로 개발 환경을 세팅할 수 있도록 한다
- 코드 스타일, 브랜치 전략, 커밋 메시지, PR 규칙을 통일하여 코드 품질을 유지한다

## 개발 환경 세팅

### Common

- Prerequisites
  - Git
  - CMake
  - Ninja
  - Clang/LLVM
  - clang-format
  - clang-tidy
  - cppcheck
  - VSCode
  
### macOS

- Prerequisites
  - Homebrew
  - Xcode Command Line Tools

```bash
brew install llvm cmake ninja cppcheck

# llvm PATH 설정이 안되어있다면...
export PATH="/opt/homebrew/opt/llvm/bin:$PATH"

# 혹은 ~/.zshrc에 추가
echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
source ~/.zshrc
```

### Ubuntu(WSL)

```bash
sudo apt-get update
sudo apt-get install -y clang llvm clang-tidy clang-format cmake ninja-build cppcheck
```

## 빌드 및 테스트

### dev 빌드
```bash
cmake --preset dev
cmake --build --preset dev
```

### 테스트 실행
```bash
ctest --preset dev
```

### 커버리지 리포트 생성
```bash
./scripts/coverage.sh
```

## Code Style & Static Analysis

### Format

- `.clang-format` 기준으로 formatting
- 되도록이면 Format on Save 사용

```bash
# 전체 파일 포맷 체크만
./scripts/format.sh check
```

```bash
# 전체 파일 수정
./scripts/format.sh fix
```

### Lint

- `.clang-tidy` 기준으로 lint

```bash
./scripts/lint.sh
```

### cppcheck

```bash
./scripts/run-cppcheck.sh
```

## Directory Structure

```
rvc-controller/
├── src/                            # implementation
├── include/                        # public header
├── tests/                          # test codes
└── scripts/                        # helper script
    ├── coverage.sh                 # 커버리지 리포트 생성
    ├── format.sh                   # 코드 포맷팅
    ├── lint.sh                     # 코드 린팅
    └── run-cppcheck.sh             # cppcheck 실행
```

## Branch Strategy

### Branch Naming Convention

- `main`: Stable (PR merge only)
- `develop`: 개발 branch
- `feat/기능명`: 기능 개발
- `fix/버그명`: 버그 수정
- `refactor/대상`: 리팩토링
- `chore/작업명`: 빌드/CI 설정, 패키지 관리 등
- `docs/문서명`: 문서 작성
- 등등...

### Workflow

1. fetch/pull latest `develop` branch
2. create new branch from `develop`
3. work on branch(feat, fix, refactor, ...)
4. commit changes
5. push branch
6. create PR to `develop`
7. review and merge
8. delete branch

- main은 stable(배포)만, 실제 개발은 develop에서 진행
- develop에서 PR을 통해 main으로 merge

## Commit Convention

[Conventional Commits](https://www.conventionalcommits.org/)을 따름

```
<type>: <description>

feat:     새 기능
fix:      버그 수정
refactor: 리팩토링
test:     테스트 추가/수정
docs:     문서
build:    빌드 설정
ci:       CI/CD 설정
style:    코드 스타일 (포맷팅)
chore:    기타 잡일
```

e.g. `feat: add dust sensor power-up logic`

## PR

- PR Template 사용

### Checklist

- [ ] `./scripts/format.sh check` 통과
- [ ] `./scripts/lint.sh` 통과
- [ ] `./scripts/run-cppcheck.sh` 경고 없음
- [ ] 테스트 추가/수정 완료 (`ctest --preset dev` 통과)
- [ ] 커밋 메시지가 Conventional Commits 규칙을 따름

## 코드 리뷰

- 모든 PR은 최소 1명의 리뷰어 승인 필요
- CI 통과 시에만 merge 가능
- comment 달리면 수정 후 다시 push
