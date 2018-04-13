%%% Document class for "Computación y Sistemas" journal
%%% Developed by Gastón Hugo SALAZAR-SILVA <ghsalazar@ipn.mx>
%%% Adds by Alexander Gelbukh 
%%% Biography Section is taken from IEEE style

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{cys}[2012/12/12 V1.0 by Gaston, Alexander & Grigori]

% class option conditionals
\newif\ifCLASSOPTIONspanish\CLASSOPTIONspanishfalse
\DeclareOption{spanish}{\CLASSOPTIONspanishtrue\PassOptionsToPackage{spanish}{babel}}
\ProcessOptions

\LoadClass[10pt,letterpaper,twocolumn]{article}

\RequirePackage[small,raggedright]{titlesec}
\RequirePackage{authblk}
\RequirePackage[font=small,labelfont=bf,labelsep=period]{caption}
\RequirePackage{geometry}
\RequirePackage{flushend}
\RequirePackage{helvet}
\RequirePackage{sansmath}

\def\CyS@labelitemi{---}

\def\labelitemi{\CyS@labelitemi}

%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%% Spanish
%%%%%%%%%%%%%%%%%%%%%

\ifCLASSOPTIONspanish
\RequirePackage{babel}
\addto\captionsspanish{%
  \def\listtablename{\'Indice de tablas}%
  \def\tablename{Tabla}}
  \es@itemize
    {\CyS@labelitemi}%
    {\textbullet}%
    {$\m@th\circ$}%
    {$\m@th\diamond$}
\fi

%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%% Bios
%%%%%%%%%%%%%%%%%%

\newif\ifCLASSOPTIONcompsoc\CLASSOPTIONcompsocfalse
%\DeclareOption{compsoc}{\CLASSOPTIONcompsoctrue}

%%% Document format

\setlength{\parskip}{0.2mm plus5mm minus0.1mm}
\setlength{\columnsep}{.36in}
%\geometry{tmargin=1.38in,bmargin=.98in,lmargin=.98in,rmargin=.98in,headheight=1.25cm,footskip=1.25cm}
\geometry{tmargin=3.5cm,bmargin=4cm,lmargin=2.5cm,rmargin=2.5cm,headheight=2.7cm,footskip=2.5cm}
\renewcommand{\familydefault}{\sfdefault}
\pagestyle{empty}

\widowpenalty=1000
\clubpenalty=1000
\let\CYShyphenpenalty\hyphenpenalty
\let\CYSexhyphenpenalty\exhyphenpenalty
\hyphenpenalty=10000
\exhyphenpenalty=10000

\renewcommand{\figurename}{Fig.} 

%%% Title format

\renewcommand{\title}[2][\relax]{%
  \gdef\@shorttile{#1}%
  \gdef\@title{#2}}

\newcommand{\othertitle}[1]{%
\vspace{4pt}
\begin{center}
\textbf{\large #1}
\end{center}
\vspace{-12pt}}

\renewcommand{\maketitle}{\twocolumn[\@maketitle]}

\renewcommand{\@maketitle}{%
%  \null\vfil
%  \vskip 60\p@
%  \vspace{-1cm}
  \begin{center}%
    {\Large\bf \@title}%
    %\par{\Large \par}%
    \noindent%
    %\vspace{1mm}\textbf{\textit{\large\@otitle}}%
    %\par{\large \par}%
    \vskip 18pt%
    \begin{tabular}[t]{c}%
      \@author\\%
    \end{tabular}
%    \small%
%    \begin{tabular}[t]{c}%
%      \@affiliation%
%    \end{tabular}\\[6pt]%
%    \textsl{\underbar{\@date}}%
  \end{center}\par%
  \@thanks
}

\renewenvironment{abstract}%
{\vspace{.19in}\noindent \textbf{\small Abstract.}\small}%
{{\small \par}}

\newenvironment{resumen}%
{\vspace{.19in}\noindent \textbf{\small Resumen.}\small}%
{{\small \par}}

\newenvironment{keywords}{\vspace{.08in}\noindent\textbf{\small Keywords.}\small}{\small\par}
\newenvironment{palabrasclave}{\vspace{.08in}\noindent\textbf{\small Palabras clave.}\small}{\small\par}

%%% Section format

\titlelabel{\thetitle\ }

%\titleformat{\section}{\normalfont\Large\bfseries\raggedright}{\thesection}{1em}{}
%\titleformat{\subsection}{\normalfont\large\bfseries\justifyheading}{\thesubsection}{1em}{}

%%% References format

\makeatletter
\renewcommand{\@biblabel}[1]{\bf #1.}
\makeatother


%%% Biographies format

\makeatletter
\long\def\@IEEEcompsoconly#1{\relax\ifCLASSOPTIONcompsoc\relax#1\relax\fi\relax}

\newdimen\@IEEEtrantmpdimenA
\newdimen\@IEEEtrantmpdimenB
\newcount\@IEEEtrantmpcountA
\newcount\@IEEEtrantmpcountB

\begingroup
\catcode`\Q=3
\long\gdef\@ifmtarg#1{\@xifmtarg#1QQ\@secondoftwo\@firstoftwo\@nil}
\long\gdef\@xifmtarg#1#2Q#3#4#5\@nil{#4}
\endgroup

\def\@IEEEtranneedspace#1#2{\penalty-100\begingroup%shield temp variable
\@IEEEtrantmpdimenB\pagegoal\advance\@IEEEtrantmpdimenB-\pagetotal% space left
\ifdim #1>\@IEEEtrantmpdimenB\relax% not enough space left
\ifdim\@IEEEtrantmpdimenB>\z@\relax #2\fi%
\newpage%
\fi\endgroup}

\long\def\@IEEEgobbleleadPARNLSP#1{\let\@IEEEswallowthistoken=0%
\let\@IEEEgobbleleadPARNLSPtoken#1%
\let\@IEEEgobbleleadPARtoken=\par%
\let\@IEEEgobbleleadNLtoken=\\%
\let\@IEEEgobbleleadSPtoken=\ %
\def\@IEEEgobbleleadSPMACRO{\ }%
\ifx\@IEEEgobbleleadPARNLSPtoken\@IEEEgobbleleadPARtoken%
\let\@IEEEswallowthistoken=1%
\fi%
\ifx\@IEEEgobbleleadPARNLSPtoken\@IEEEgobbleleadNLtoken%
\let\@IEEEswallowthistoken=1%
\fi%
\ifx\@IEEEgobbleleadPARNLSPtoken\@IEEEgobbleleadSPtoken%
\let\@IEEEswallowthistoken=1%
\fi%
% a control space will come in as a macro
% when it is the last one on a line
\ifx\@IEEEgobbleleadPARNLSPtoken\@IEEEgobbleleadSPMACRO%
\let\@IEEEswallowthistoken=1%
\fi%
% if we have to swallow this token, do so and taste the next one
% else spit it out and stop gobbling
\ifx\@IEEEswallowthistoken 1\let\@IEEEnextgobbleleadPARNLSP=\@IEEEgobbleleadPARNLSP\else%
\let\@IEEEnextgobbleleadPARNLSP=#1\fi%
\@IEEEnextgobbleleadPARNLSP}%

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% IEEEbiography ENVIRONMENT
% Allows user to enter biography leaving place for picture (adapts to font size)
% As of V1.5, a new optional argument allows you to have a real graphic!
% V1.5 and later also fixes the "colliding biographies" which could happen when a 
% biography's text was shorter than the space for the photo.
% MDS 7/2001
% V1.6 prevent multiple biographies from making multiple TOC entries
\newif\if@IEEEbiographyTOCentrynotmade
\global\@IEEEbiographyTOCentrynotmadetrue

% biography counter so hyperref can jump directly to the biographies
% and not just the previous section
\newcounter{IEEEbiography}
\setcounter{IEEEbiography}{0}

% photo area size
\def\@IEEEBIOphotowidth{1.0in}    % width of the biography photo area
\def\@IEEEBIOphotodepth{1.25in}   % depth (height) of the biography photo area
% area cleared for photo
\def\@IEEEBIOhangwidth{1.14in}    % width cleared for the biography photo area
\def\@IEEEBIOhangdepth{1.25in}    % depth cleared for the biography photo area
                                  % actual depth will be a multiple of 
                                  % \baselineskip, rounded up
\def\@IEEEBIOskipN{4\baselineskip}% nominal value of the vskip above the biography

\newenvironment{IEEEbiography}[2][]{\normalfont\@IEEEcompsoconly{\sffamily}\normalsize %\footnotesize%
\unitlength 1in\parskip=0pt\par\parindent 1em\interlinepenalty500%
% we need enough space to support the hanging indent
% the nominal value of the spacer
% and one extra line for good measure
\@IEEEtrantmpdimenA=\@IEEEBIOhangdepth%
\advance\@IEEEtrantmpdimenA by \@IEEEBIOskipN%
\advance\@IEEEtrantmpdimenA by 1\baselineskip%
% if this page does not have enough space, break it and lets start
% with a new one
\@IEEEtranneedspace{\@IEEEtrantmpdimenA}{\relax}%
% nominal spacer can strech, not shrink use 1fil so user can out stretch with \vfill
\vskip \@IEEEBIOskipN plus 1fil minus 0\baselineskip%
% the default box for where the photo goes
\def\@IEEEtempbiographybox{{\setlength{\fboxsep}{0pt}\framebox{%
\begin{minipage}[b][\@IEEEBIOphotodepth][c]{\@IEEEBIOphotowidth}\centering PLACE\\ PHOTO\\ HERE \end{minipage}}}}%
%
% detect if the optional argument was supplied, this requires the
% \@ifmtarg command as defined in the appendix section above
% and if so, override the default box with what they want
\@ifmtarg{#1}{\relax}{\def\@IEEEtempbiographybox{\mbox{\begin{minipage}[b][\@IEEEBIOphotodepth][c]{\@IEEEBIOphotowidth}%
\centering%
#1%
\end{minipage}}}}% end if optional argument supplied
% Make an entry into the table of contents only if we have not done so before
\if@IEEEbiographyTOCentrynotmade%
% link labels to the biography counter so hyperref will jump
% to the biography, not the previous section
\setcounter{IEEEbiography}{-1}%
\refstepcounter{IEEEbiography}%
%%%%% AFG: for some reason this prevents the paragraph from splitting to new column %%%%% \addcontentsline{toc}{section}{Biographies}%
\global\@IEEEbiographyTOCentrynotmadefalse%
\fi%
% one more biography
\refstepcounter{IEEEbiography}%
% Make an entry for this name into the table of contents 
%%%%% AFG: for some reason this prevents the paragraph from splitting to new column %%%%% \addcontentsline{toc}{subsection}{#2}%
% V1.6 properly handle if a new paragraph should occur while the
% hanging indent is still active. Do this by redefining \par so
% that it will not start a new paragraph. (But it will appear to the
% user as if it did.) Also, strip any leading pars, newlines, or spaces.
\let\@IEEEBIOORGparCMD=\par% save the original \par command
\edef\par{\hfil\break\indent}% the new \par will not be a "real" \par
\settoheight{\@IEEEtrantmpdimenA}{\@IEEEtempbiographybox}% get height of biography box
\@IEEEtrantmpdimenB=\@IEEEBIOhangdepth%
\@IEEEtrantmpcountA=\@IEEEtrantmpdimenB% countA has the hang depth
\divide\@IEEEtrantmpcountA by \baselineskip%  calculates lines needed to produce the hang depth
\advance\@IEEEtrantmpcountA by 1% ensure we overestimate
% set the hanging indent
\hangindent\@IEEEBIOhangwidth%
\hangafter-\@IEEEtrantmpcountA%
% reference the top of the photo area to the top of a capital T
\settoheight{\@IEEEtrantmpdimenB}{\mbox{T}}%
% set the photo box, give it zero width and height so as not to disturb anything
\noindent\makebox[0pt][l]{\hspace{-\@IEEEBIOhangwidth}\raisebox{\@IEEEtrantmpdimenB}[0pt][0pt]{%
\raisebox{-\@IEEEBIOphotodepth}[0pt][0pt]{\@IEEEtempbiographybox}}}%
% now place the author name and begin the bio text
\noindent\textbf{#2\ }\@IEEEgobbleleadPARNLSP}{\relax\let\par=\@IEEEBIOORGparCMD\par%
% 7/2001 V1.5 detect when the biography text is shorter than the photo area
% and pad the unused area - preventing a collision from the next biography entry
% MDS
\ifnum \prevgraf <\@IEEEtrantmpcountA\relax% detect when the biography text is shorter than the photo
    \advance\@IEEEtrantmpcountA by -\prevgraf% calculate how many lines we need to pad
    \advance\@IEEEtrantmpcountA by -1\relax% we compensate for the fact that we indented an extra line
    \@IEEEtrantmpdimenA=\baselineskip% calculate the length of the padding
    \multiply\@IEEEtrantmpdimenA by \@IEEEtrantmpcountA%
    \noindent\rule{0pt}{\@IEEEtrantmpdimenA}% insert an invisible support strut
\fi%
\par\normalfont}
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
\makeatother

\newenvironment{biography}[2][]{\begin{IEEEbiography}[{\includegraphics[width=1in,height=1.25in,clip,keepaspectratio]{#1}}]{\bf #2}}{\end{IEEEbiography}}

%\newenvironment{biography}[1]%
%{\noindent\textbf{#1}}%
%{\vskip 12pt}

\hbadness=9999
\vbadness=9999
