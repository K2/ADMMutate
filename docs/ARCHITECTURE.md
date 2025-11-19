# Architecture Guide

## System Architecture

```mermaid
%%{init: {'theme':'dark'}}%%
graph TB
    subgraph Input["🎯 Input Layer"]
        A[Exploit Developer]
        B[Shellcode]
        C[Configuration]
    end
    
    subgraph API["⚙️ API Layer (ADMmutapi.h)"]
        D[init_mutate]
        E[apply_key]
        F[apply_jnops]
        G[apply_engine]
        H[apply_offset_mod]
    end
    
    subgraph Engine["🔧 Mutation Engine (ADMmuteng.c)"]
        I[Key Search Algorithm]
        J[Encoder/Decoder Generator]
        K[NOP Substitution Engine]
        L[Architecture Handler]
    end
    
    subgraph Arch["💻 Architecture Support"]
        M[IA32 Handler]
        N[SPARC Handler]
        O[HPPA Handler]
        P[MIPS Handler]
    end
    
    subgraph Output["📦 Output Layer"]
        Q[Polymorphic Buffer]
        R[Unique Shellcode]
        S[Runtime Decoder]
    end
    
    A --> D
    B --> E
    C --> D
    
    D --> L
    E --> I
    E --> J
    F --> K
    G --> J
    H --> L
    
    L --> M
    L --> N
    L --> O
    L --> P
    
    I --> Q
    J --> R
    J --> S
    K --> Q
    
    Q --> T[💣 Exploit Payload]
    R --> T
    S --> T
    
    style Input fill:#1a1a2e,stroke:#16213e,color:#fff
    style API fill:#0f3460,stroke:#16213e,color:#fff
    style Engine fill:#16213e,stroke:#0f3460,color:#fff
    style Arch fill:#533483,stroke:#7209b7,color:#fff
    style Output fill:#1a472a,stroke:#2d7a45,color:#fff
    
    style D fill:#e94560,stroke:#c72c48,color:#fff
    style E fill:#e94560,stroke:#c72c48,color:#fff
    style F fill:#e94560,stroke:#c72c48,color:#fff
    style G fill:#e94560,stroke:#c72c48,color:#fff
    style H fill:#e94560,stroke:#c72c48,color:#fff
    
    style T fill:#388e3c,stroke:#2e7d32,color:#fff
```

## Encoding Process Flow

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#1a1a2e','primaryTextColor':'#fff','primaryBorderColor':'#e94560','lineColor':'#16a085'}}}%%
stateDiagram-v2
    [*] --> Initialize: init_mutate()
    
    Initialize --> CheckConstraints: Configuration Set
    CheckConstraints --> GenerateKeyspace: Parse banned chars,<br/>toupper/tolower flags
    
    GenerateKeyspace --> FilterKeys: Generate 2^16 keys
    FilterKeys --> TestKeys: Remove banned<br/>combinations
    
    TestKeys --> EncodeShellcode: Find valid key<br/>(XOR encoding)
    EncodeShellcode --> VerifyCompliance: Encode with<br/>selected key
    
    VerifyCompliance --> EncodeShellcode: ❌ Failed constraints
    VerifyCompliance --> SaveKey: ✅ Valid encoding
    
    SaveKey --> MoreKeys: Store valid key
    MoreKeys --> EncodeShellcode: Test more keys
    MoreKeys --> SelectRandom: Enough keys found
    
    SelectRandom --> ApplyEncoding: Random selection<br/>from valid keys
    ApplyEncoding --> [*]: Shellcode Encoded
    
    note right of GenerateKeyspace
        Keyspace: 2x16-bit = 32-bit
        IA32_SLIDE: +32-bit slide = 64-bit
    end note
    
    note right of VerifyCompliance
        Checks:
        • No banned chars
        • toupper/tolower safe
        • NULL-free if required
    end note
```

## Decoder Generation Architecture

```mermaid
%%{init: {'theme':'dark'}}%%
flowchart LR
    subgraph Input[" 🎲 Random Components"]
        A1[Instruction Order]
        A2[Code Path Selection]
        A3[Junk Padding]
        A4[Register Selection]
    end
    
    subgraph Generator["🏗️ Decoder Builder"]
        B1[Select Instructions<br/>from ALTERNATE paths]
        B2[Insert Junk Pads<br/>JMAX operations]
        B3[Add Dynamic Values<br/>Key, Slide, Offsets]
        B4[Out-of-Order<br/>Assembly]
    end
    
    subgraph Components["📝 Decoder Components"]
        C1[Key Loading<br/>multiple methods]
        C2[Counter Setup<br/>shellcode length]
        C3[Decode Loop<br/>XOR operations]
        C4[Increment Logic<br/>pointer + counter]
    end
    
    subgraph Output["⚡ Runtime Decoder"]
        D1[Polymorphic Code]
        D2[✅ Unique Every Time]
    end
    
    A1 --> B1
    A2 --> B1
    A3 --> B2
    A4 --> B3
    
    B1 --> B4
    B2 --> B4
    B3 --> B4
    
    B4 --> C1
    B4 --> C2
    B4 --> C3
    B4 --> C4
    
    C1 --> D1
    C2 --> D1
    C3 --> D1
    C4 --> D1
    
    D1 --> D2
    
    style Input fill:#1a1a2e,stroke:#16213e,color:#fff
    style Generator fill:#16213e,stroke:#e94560,color:#fff
    style Components fill:#0f3460,stroke:#16a085,color:#fff
    style Output fill:#1a472a,stroke:#2d7a45,color:#fff
```

## NOP Replacement Strategy

```mermaid
%%{init: {'theme':'base', 'themeVariables': {'primaryColor':'#1a1a2e','primaryTextColor':'#fff','lineColor':'#16a085'}}}%%
graph TD
    A[Traditional NOP Sled<br/>0x90 0x90 0x90...] --> B{ADMmutate}
    
    B --> C[Filter by Constraints]
    C --> D[55 IA32 Alternatives]
    
    D --> E1[Push/Pop Operations<br/>0x50-0x5F]
    D --> E2[Exchange Instructions<br/>0x90-0x97]
    D --> E3[Inc/Dec Operations<br/>0x40-0x4F]
    D --> E4[Arithmetic NOPs<br/>XOR, TEST, etc]
    D --> E5[Dynamic Instructions<br/>SHR, IMUL, ROL]
    
    E1 --> F[Weight Selection]
    E2 --> F
    E3 --> F
    E4 --> F
    E5 --> F
    
    F --> G[Statistical Distribution]
    G --> H[Polymorphic NOP Sled<br/>✅ Unique Pattern]
    
    style A fill:#6b2c2c,stroke:#8b3a3a,color:#fff
    style B fill:#e94560,stroke:#c72c48,color:#fff
    style H fill:#388e3c,stroke:#2e7d32,color:#fff
    
    style E1 fill:#0f3460,stroke:#16213e,color:#fff
    style E2 fill:#0f3460,stroke:#16213e,color:#fff
    style E3 fill:#0f3460,stroke:#16213e,color:#fff
    style E4 fill:#0f3460,stroke:#16213e,color:#fff
    style E5 fill:#0f3460,stroke:#16213e,color:#fff
```

## Architecture-Specific Handlers

```mermaid
%%{init: {'theme':'dark'}}%%
classDiagram
    class MorphEngine {
        +struct junks[] njunk
        +struct decoder[] morpheng
        +init_mutate()
        +apply_key()
        +apply_engine()
        +apply_jnops()
    }
    
    class IA32Handler {
        +55 junk instructions
        +13 decoder instructions
        +TSC register seeding
        +mutate_fixup_ia32()
    }
    
    class IA32SlideHandler {
        +14 decoder instructions
        +Sliding key support
        +64-bit keyspace
        +mutate_slide()
    }
    
    class SPARCHandler {
        +21 junk instructions
        +15 decoder instructions
        +TICK register seeding
        +mutate_fixup_sparc()
    }
    
    class HPPAHandler {
        +11 junk instructions
        +15 decoder instructions
        +TIMER register seeding
        +Cache flush handling
        +fork() execution trick
        +mutate_fixup_hppa()
    }
    
    MorphEngine <|-- IA32Handler
    MorphEngine <|-- IA32SlideHandler
    MorphEngine <|-- SPARCHandler
    MorphEngine <|-- HPPAHandler
    
    note for IA32Handler "Standard x86 support\nVariable-length instructions\n1-16 bytes per opcode"
    note for IA32SlideHandler "Enhanced security\nDefeat XOR reversal\n32-bit slide increment"
    note for HPPAHandler "Separate I/D cache\nRequires flush + sync\nChild process execution"
```

## Key Search Algorithm

```mermaid
%%{init: {'theme':'base', 'themeVariables': { 'primaryColor':'#16213e','secondaryColor':'#1a1a2e','tertiaryColor':'#0f3460'}}}%%
flowchart TD
    Start([Start Key Search]) --> Init[Initialize:<br/>keyspace = 2^16<br/>good_keys array]
    
    Init --> Parse[Parse Constraints]
    Parse --> Banned{Banned<br/>chars?}
    Banned -->|Yes| RemoveBanned[Remove keys with<br/>banned chars]
    Banned -->|No| CheckUpper
    RemoveBanned --> CheckUpper
    
    CheckUpper{toupper<br/>mode?}
    CheckUpper -->|Yes| RemoveUpper[Remove non-uppercase<br/>compliant keys]
    CheckUpper -->|No| CheckLower
    RemoveUpper --> CheckLower
    
    CheckLower{tolower<br/>mode?}
    CheckLower -->|Yes| RemoveLower[Remove non-lowercase<br/>compliant keys]
    CheckLower -->|No| TestAll
    RemoveLower --> TestAll
    
    TestAll[Test remaining keys<br/>against shellcode]
    TestAll --> Loop{For each<br/>key}
    
    Loop --> Encode[XOR encode shellcode]
    Encode --> Verify{Encoded output<br/>compliant?}
    
    Verify -->|No| Loop
    Verify -->|Yes| Store[Store in good_keys]
    Store --> Count{Enough<br/>keys?}
    
    Count -->|No| Loop
    Count -->|Yes| Round2[Round 2 verification]
    
    Round2 --> Select[Random selection<br/>from good_keys]
    Select --> End([Return selected key])
    
    style Start fill:#1a472a,stroke:#2d7a45,color:#fff
    style End fill:#388e3c,stroke:#2e7d32,color:#fff
    style Verify fill:#e94560,stroke:#c72c48,color:#fff
    style Store fill:#16a085,stroke:#0d7a5f,color:#fff
```

## Memory Layout

```mermaid
%%{init: {'theme':'dark'}}%%
graph TB
    subgraph Buffer["💾 Exploit Buffer (1024 bytes example)"]
        direction TB
        A["<div style='text-align:left'>Offset 0<br/>↓</div>"]
        B["🔀 Junk NOP Sled<br/>(499 bytes)<br/>Polymorphic instructions"]
        C["⚙️ Decoder Engine<br/>(65-72 bytes)<br/>Dynamic decoder"]
        D["🔒 Encoded Shellcode<br/>(52 bytes)<br/>XOR encrypted"]
        E["📍 Return Addresses<br/>(408 bytes)<br/>Modulated offsets"]
        F["<div style='text-align:left'>↑<br/>Offset 1024</div>"]
    end
    
    B -.->|Slides to| C
    C -.->|Decodes| D
    D -.->|Executes| G[/bin/sh]
    E -.->|Points to| B
    
    style B fill:#0f3460,stroke:#16a085,color:#fff
    style C fill:#533483,stroke:#7209b7,color:#fff
    style D fill:#e94560,stroke:#c72c48,color:#fff
    style E fill:#d68910,stroke:#b87333,color:#fff
    style G fill:#388e3c,stroke:#2e7d32,color:#fff
```

## Runtime Execution Flow

```mermaid
%%{init: {'theme':'base', 'themeVariables': {'primaryColor':'#1a1a2e','primaryTextColor':'#fff','lineColor':'#e94560'}}}%%
sequenceDiagram
    autonumber
    
    participant V as Vulnerable Program
    participant B as Exploit Buffer
    participant D as Decoder
    participant S as Shellcode
    participant OS as Operating System
    
    rect rgb(42, 42, 42)
    note right of V: Overflow Phase
    V->>B: Buffer overflow occurs
    V->>B: Return address overwritten
    end
    
    rect rgb(83, 52, 131)
    note right of B: Execution Redirect
    B->>B: CPU jumps to NOP sled
    B->>D: Slide through junk NOPs
    D->>D: Decoder starts executing
    end
    
    rect rgb(233, 69, 96)
    note right of D: Decode Phase
    loop For each DWORD
        D->>D: Load XOR key + slide
        D->>S: XOR decode 4 bytes
        D->>D: Increment key (if sliding)
        D->>D: Move to next DWORD
    end
    end
    
    rect rgb(22, 161, 133)
    note right of S: Shellcode Execution
    S->>S: Decoded shellcode ready
    S->>OS: execve("/bin/sh")
    OS-->>S: Shell spawned
    end
    
    rect rgb(56, 142, 60)
    note right of OS: Success
    OS->>V: ✅ Shell access granted
    end
```

## HPPA Cache Flush Architecture

```mermaid
%%{init: {'theme':'dark'}}%%
stateDiagram-v2
    [*] --> WriteDecoder: Decoder writes to memory
    
    WriteDecoder --> Problem: I-cache & D-cache separate
    Problem --> CantExecute: D-cache has new code<br/>I-cache has old code
    
    CantExecute --> Solution1: Try: flush I & D cache
    Solution1 --> NeedSync: fic + fdc instructions
    NeedSync --> Bloat: Requires sync (0x00040000)<br/>4 inst + 6 NULL bytes per flush
    
    Bloat --> BadIdea: Too much bloat<br/>+ 7 instruction padding
    
    CantExecute --> Solution2: Try: Context switch
    Solution2 --> TriedSetreuid: setreuid() doesn't work
    
    CantExecute --> Solution3: ✅ fork() Trick
    Solution3 --> ParentDies: Parent process exits
    Solution3 --> ChildExecutes: Child gets flushed cache
    
    ChildExecutes --> Success: Operational image<br/>executes correctly
    Success --> [*]
    
    note right of Problem
        HPPA has Harvard architecture
        Instruction and Data caches
        are independent
    end note
    
    note right of Solution3
        Observed: coredumps had
        accurate flushed cache
        Therefore: fork() must
        copy flushed image
    end note
```

## Component Interaction Matrix

| Component | init_mutate | apply_key | apply_jnops | apply_engine | apply_offset_mod |
|-----------|-------------|-----------|-------------|--------------|------------------|
| **Configuration** | ✅ Parse | ✅ Use | ✅ Use | ✅ Use | ✅ Use |
| **Junks Array** | ✅ Load | ❌ | ✅ Select | ❌ | ❌ |
| **Decoder Array** | ✅ Load | ❌ | ❌ | ✅ Select | ❌ |
| **Key Search** | ❌ | ✅ Execute | ❌ | ❌ | ❌ |
| **Buffer Write** | ❌ | ✅ Encode | ✅ Replace | ✅ Insert | ✅ Modulate |
| **Random Gen** | ❌ | ✅ Select | ✅ Select | ✅ Generate | ✅ Offset |

```mermaid
%%{init: {'theme':'base'}}%%
pie title Component Usage Distribution
    "Key Search (32%)" : 32
    "Decoder Generation (28%)" : 28
    "NOP Replacement (20%)" : 20
    "Initialization (12%)" : 12
    "Offset Modulation (8%)" : 8
```

## Design Principles

### 🎯 Polymorphism First
Every execution must produce unique binary output while maintaining functional equivalence.

### 🔒 Security Through Obscurity
Multiple layers of randomization make pattern detection computationally expensive for NIDS.

### ⚡ Performance Conscious  
Efficient key search and caching strategies ensure rapid exploit generation.

### 🔧 Architecture Agnostic
Modular design allows easy addition of new CPU architectures.

### 📊 Statistical Distribution
Weighted selection ensures generated code matches real-world instruction frequency patterns.

---

**Previous**: [Getting Started](GETTING_STARTED.md) | **Next**: [API Reference](API_REFERENCE.md)
