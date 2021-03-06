#include "musicmodel.h"
#include <QFileInfo>

extern "C"
{
#include <libavformat/avformat.h>
}

static AVFormatContext *avformart = nullptr;

MusicData::MusicData(const QUrl &filename, QObject *parent)
    : QObject (parent)
    , m_filename(filename)
{

}

qreal MusicData::duration() const
{
    return m_duration;
}

QString MusicData::title() const
{
    return m_title;
}

QString MusicData::singer() const
{
    return m_singer;
}

QString MusicData::album() const
{
    return m_album;
}

QUrl MusicData::filename() const
{
    return m_filename;
}

MusicData* MusicData::create(const QUrl &filename, QObject *parent)
{
    int ret = avformat_open_input(&avformart, filename.toLocalFile().toStdString().c_str(), nullptr, nullptr);

    if (ret != 0) {
        avformat_close_input(&avformart);
        return nullptr;
    }

    MusicData *data = new MusicData(filename, parent);
    AVDictionaryEntry *title = av_dict_get(avformart->metadata, "title", nullptr, AV_DICT_MATCH_CASE);
    AVDictionaryEntry *artist = av_dict_get(avformart->metadata, "artist", nullptr, AV_DICT_MATCH_CASE);
    AVDictionaryEntry *album = av_dict_get(avformart->metadata, "album", nullptr, AV_DICT_MATCH_CASE);
    if (album) data->m_album = album->value;
    if (artist) data->m_singer = artist->value;
    if (title) data->m_title = title->value;
    else data->m_title = QFileInfo(filename.toLocalFile()).baseName();

    avformat_find_stream_info(avformart, nullptr);
    int streamIdx = av_find_best_stream(avformart, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
    if (ret < 0) {
        data->m_duration = 0.0;
    } else {
        AVStream *stream = avformart->streams[streamIdx];
        data->m_duration = stream->duration * av_q2d(stream->time_base);
    }

    avformat_close_input(&avformart);

    return data;
}

MusicModel::MusicModel(QObject *parent)
    : QObject (parent)
{

}

void MusicModel::sort(SortKey key, SortMode mode)
{
    switch (key) {
    case SortKey::Title:
        if (mode == SortMode::Less) {
            std::sort(m_list.begin(), m_list.end(), [](MusicData *d1, MusicData *d2)->bool {
                return d1->m_title < d2->m_title;
            });
        } else {
            std::sort(m_list.begin(), m_list.end(), [](MusicData *d1, MusicData *d2)->bool {
                return d1->m_title > d2->m_title;
            });
        }
        break;
    case SortKey::Duration:
        if (mode == SortMode::Less) {
            std::sort(m_list.begin(), m_list.end(), [](MusicData *d1, MusicData *d2)->bool {
                return d1->m_duration < d2->m_duration;
            });
        } else {
            std::sort(m_list.begin(), m_list.end(), [](MusicData *d1, MusicData *d2)->bool {
                return d1->m_duration > d2->m_duration;
            });
        }
        break;
    case SortKey::Singer:
        if (mode == SortMode::Less) {
            std::sort(m_list.begin(), m_list.end(), [](MusicData *d1, MusicData *d2)->bool {
                return d1->m_singer < d2->m_singer;
            });
        } else {
            std::sort(m_list.begin(), m_list.end(), [](MusicData *d1, MusicData *d2)->bool {
                return d1->m_singer > d2->m_singer;
            });
        }
        break;
    case SortKey::Album:
        if (mode == SortMode::Less) {
            std::sort(m_list.begin(), m_list.end(), [](MusicData *d1, MusicData *d2)->bool {
                return d1->m_album < d2->m_album;
            });
        } else {
            std::sort(m_list.begin(), m_list.end(), [](MusicData *d1, MusicData *d2)->bool {
                return d1->m_album > d2->m_album;
            });
        }
        break;
    }

    emit modelChanged();
}

QQmlListProperty<MusicData> MusicModel::model()
{
    return QQmlListProperty<MusicData>(this, this,
                                       &MusicModel::append,
                                       &MusicModel::count,
                                       &MusicModel::at,
                                       &MusicModel::clear);
}

void MusicModel::setModel(const QVector<MusicData *> &music)
{
    m_list.clear();
    m_list = music;
    emit modelChanged();
}

int MusicModel::indexof(MusicData *const &music)
{
    return m_list.indexOf(music);
}

void MusicModel::append(MusicData *music)
{
    return m_list.append(music);
}

int MusicModel::count() const
{
    return m_list.count();
}

MusicData* MusicModel::at(int index)
{
    return m_list.at(index);
}

void MusicModel::clear()
{
    for (auto it : m_list) it->deleteLater();
    return m_list.clear();
}

void MusicModel::append(QQmlListProperty<MusicData> *list, MusicData *music)
{
    return reinterpret_cast<MusicModel*>(list->data)->append(music);
}

int MusicModel::count(QQmlListProperty<MusicData> *list)
{
    return reinterpret_cast<MusicModel*>(list->data)->count();
}

MusicData* MusicModel::at(QQmlListProperty<MusicData> *list, int index)
{
    return reinterpret_cast<MusicModel*>(list->data)->at(index);
}

void MusicModel::clear(QQmlListProperty<MusicData> *list)
{
    return reinterpret_cast<MusicModel*>(list->data)->clear();
}
